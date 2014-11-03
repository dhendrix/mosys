/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * cros_ec_lpc.c: Subset of Google LPC EC interface functionality (ported from
 * chromium os repo)
 */

#include <inttypes.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/cros_ec.h"
#include "drivers/google/cros_ec_commands.h"
#include "drivers/google/cros_ec_lock.h"

#include "intf/io.h"

#include "lib/math.h"

/* LPC command status byte masks */
/* EC has written a byte in the data register and host hasn't read it yet */
/* Host has written a command/data byte and the EC hasn't read it yet */
#define CROS_EC_STATUS_FROM_HOST   0x02
/* EC is processing a command */
#define CROS_EC_STATUS_PROCESSING  0x04
/* Last write to EC was a command, not data */
#define CROS_EC_STATUS_LAST_CMD    0x08
/* EC is in burst mode.  Chrome EC doesn't support this, so this bit is never
 * set. */
#define CROS_EC_STATUS_BURST_MODE  0x10
/* SCI event is pending (requesting SCI query) */
#define CROS_EC_STATUS_SCI_PENDING 0x20
/* SMI event is pending (requesting SMI query) */
#define CROS_EC_STATUS_SMI_PENDING 0x40
/* (reserved) */
#define CROS_EC_STATUS_RESERVED    0x80

/* Waits for the EC to be unbusy.  Returns 0 if unbusy, non-zero if
 * timeout. */
static int wait_for_ec(struct platform_intf *intf,
                       int status_addr, int timeout_usec)
{
	int i;
	uint8_t data;

	for (i = 0; i < timeout_usec; i += 10) {
		usleep(10);  /* Delay first, in case we just sent a command */

		if (io_read8(intf, status_addr, &data))
			return -1;

		if (!(data & EC_LPC_STATUS_BUSY_MASK))
			return 0;
	}

	return -1;  /* Timeout */
}

/*
 **************************** EC API v3 ****************************
 */
static int cros_ec_command_lpc_v3(struct platform_intf *intf,
				  struct ec_cb *ec,
				  int command, int command_version,
				  const void *indata, int insize,
				  const void *outdata, int outsize)
{
	struct cros_ec_priv *priv = ec->priv;
	struct ec_host_request rq;
	struct ec_host_response rs;
	const uint8_t *d;
	uint8_t *dout;
	uint8_t out;
	int csum = 0;
	int i;

	/* Fail if output size is too big */
	if (outsize + sizeof(rq) > EC_LPC_HOST_PACKET_SIZE)
		return -1;

	/* Fill in request packet */
	/* TODO(crosbug.com/p/23825): This should be common to all protocols */
	rq.struct_version = EC_HOST_REQUEST_VERSION;
	rq.checksum = 0;
	rq.command = command | EC_CMD_PASSTHRU_OFFSET(priv->device_index);
	rq.command_version = command_version;
	rq.reserved = 0;
	rq.data_len = outsize;

	/* Copy data and start checksum */
	for (i = 0, d = (const uint8_t *)outdata; i < outsize; i++, d++) {
		io_write8(intf, EC_LPC_ADDR_HOST_PACKET + sizeof(rq) + i, *d);
		csum += *d;
	}

	/* Finish checksum */
	for (i = 0, d = (const uint8_t *)&rq; i < sizeof(rq); i++, d++)
		csum += *d;

	/* Write checksum field so the entire packet sums to 0 */
	rq.checksum = (uint8_t)(-csum);

	/* Copy header */
	for (i = 0, d = (const uint8_t *)&rq; i < sizeof(rq); i++, d++)
		io_write8(intf, EC_LPC_ADDR_HOST_PACKET + i, *d);

	/* Start the command */
	io_write8(intf, EC_LPC_ADDR_HOST_CMD, EC_COMMAND_PROTOCOL_3);

	if (wait_for_ec(intf, EC_LPC_ADDR_HOST_CMD, 1000000)) {
		lprintf(LOG_ERR, "Timeout waiting for EC response\n");
		return -1;
	}

	/* Check result */
	if (io_read8(intf, EC_LPC_ADDR_HOST_DATA, &out))
		return -1;
	if (out) {
		lprintf(LOG_ERR, "EC returned error result code %d\n", i);
		return -i;
	}

	/* Read back response header and start checksum */
	csum = 0;
	for (i = 0, dout = (uint8_t *)&rs; i < sizeof(rs); i++, dout++) {
		if (io_read8(intf, EC_LPC_ADDR_HOST_PACKET + i, dout))
			return -1;
		csum += *dout;
	}

	if (rs.struct_version != EC_HOST_RESPONSE_VERSION) {
		lprintf(LOG_ERR, "EC response version mismatch\n");
		return -1;
	}

	if (rs.reserved) {
		lprintf(LOG_ERR, "EC response reserved != 0\n");
		return -1;
	}

	if (rs.data_len > insize) {
		lprintf(LOG_ERR, "EC returned too much data\n");
		return -1;
	}

	/* Read back data and update checksum */
	for (i = 0, dout = (uint8_t *)indata; i < rs.data_len; i++, dout++) {
		if (io_read8(intf, EC_LPC_ADDR_HOST_PACKET + sizeof(rs) + i, dout))
			return -1;
		csum += *dout;
	}

	/* Verify checksum */
	if ((uint8_t)csum) {
		lprintf(LOG_ERR, "EC response has invalid checksum\n");
		return -1;
	}

	/* Return actual amount of data received */
	return 0;
}

/* Sends a versioned command to the EC.  Returns the command status code,
 * or -1 if other error. */
static int cros_ec_command_lpc_v1(struct platform_intf *intf,
				  struct ec_cb *ec_unused,
			          int command, int command_version,
			          const void *indata, int insize,
			          const void *outdata, int outsize)
{
	struct ec_lpc_host_args args = {
		.flags = EC_HOST_ARGS_FLAG_FROM_HOST,
		.command_version = command_version,
		.data_size = outsize,
	};
	uint8_t *d;
	int csum, i;
	uint8_t ec_response;

	/* Initialize checksum */
	csum = command + args.flags + args.command_version + args.data_size;

	if (wait_for_ec(intf, EC_LPC_ADDR_HOST_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC ready\n",
		                   __func__);
		return -1;
	}

	/* Write data and update checksum */
	for (i = 0, d = (uint8_t *)outdata; i < outsize; i++, d++) {
		if (io_write8(intf, EC_LPC_ADDR_HOST_PARAM + i, *d))
			return -1;
		csum += *d;
	}

	/* Finalize checksum and write args */
	args.checksum = (uint8_t)csum;
	for (i = 0, d = (uint8_t *)&args; i < sizeof(args); i++, d++) {
		if (io_write8(intf, EC_LPC_ADDR_HOST_ARGS + i, *d))
			return -1;
	}

	/* Issue the command */
	if (io_write8(intf, EC_LPC_ADDR_HOST_CMD, command))
		return -1;

	if (wait_for_ec(intf, EC_LPC_ADDR_HOST_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC response\n",
		                   __func__);
		return -1;
	}

	/* Check result */
	if (io_read8(intf, EC_LPC_ADDR_HOST_DATA, &ec_response))
		return -1;

	if (ec_response) {
		lprintf(LOG_DEBUG, "%s: EC returned error result code %d\n",
		                   __func__, ec_response);
		return -1;
	}

	/* Read back args */
	for (i = 0, d = (uint8_t *)&args; i < sizeof(args); i++, d++) {
		if (io_read8(intf, EC_LPC_ADDR_HOST_ARGS + i, d))
			return -1;
	}

	/*
	 * If EC didn't modify args flags, then somehow we sent a new-style
	 * command to an old EC, which means it would have read its params
	 * from the wrong place.
	 */
	if (!(args.flags & EC_HOST_ARGS_FLAG_TO_HOST)) {
		lprintf(LOG_DEBUG, "%s: EC protocol mismatch\n", __func__);
		return -1;
	}

	if (args.data_size > insize) {
		lprintf(LOG_DEBUG, "%s: EC returned too much data\n", __func__);
		return -1;
	}
	insize = args.data_size;

	/* Start calculating response checksum */
	csum = command + args.flags + args.command_version + args.data_size;

	/* Read data, if any */
	for (i = 0, d = (uint8_t *)indata; i < insize; i++, d++) {
		if (io_read8(intf, EC_LPC_ADDR_HOST_PARAM + i, d))
			return -1;
		csum += *d;
	}

	/* Verify checksum */
	if (args.checksum != (uint8_t)csum) {
		lprintf(LOG_DEBUG, "%s: EC response has invalid checksum\n",
			__func__);
		return -1;
	}

	return ec_response;
}

/* Sends a command to the EC.  Returns the command status code, or
 * -1 if other error. */
static int cros_ec_command_lpc_v0(struct platform_intf *intf,
				  struct ec_cb *ec_unused,
			          int command, int command_version,
			          const void *indata, int insize,
			          const void *outdata, int outsize)
{
	uint8_t *d;
	int i;
	uint8_t ec_response;

	if (wait_for_ec(intf, EC_LPC_ADDR_HOST_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC ready\n",
		                   __func__);
		return -1;
	}

	/* Write data, if any */
	for (i = 0, d = (uint8_t *)outdata; i < outsize; i++, d++) {
		if (io_write8(intf, EC_LPC_ADDR_OLD_PARAM + i, *d))
			return -1;
	}

	if (io_write8(intf, EC_LPC_ADDR_HOST_CMD, command))
		return -1;

	if (wait_for_ec(intf, EC_LPC_ADDR_HOST_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC response\n",
		                   __func__);
		return -1;
	}

	/* Check result */
	if (io_read8(intf, EC_LPC_ADDR_HOST_DATA, &ec_response))
		return -1;

	if (ec_response) {
		lprintf(LOG_DEBUG, "%s: EC returned error result code %d\n",
		                   __func__, ec_response);
		return -1;
	}

	/* Read data, if any */
	for (i = 0, d = (uint8_t *)indata; i < insize; i++, d++) {
		if (io_read8(intf, EC_LPC_ADDR_OLD_PARAM + i, d))
			return -1;
	}

	return ec_response;
}

/* Check to see if versioned commands are supported by the EC */
static int cros_ec_command_lpc_detect(struct platform_intf *intf)
{
	struct cros_ec_priv *priv = intf->cb->ec->priv;
	uint8_t id1, id2, flags;

	if (priv->raw)
		return 0;

	if (io_read8(intf, EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID, &id1))
		return -1;
	if (io_read8(intf, EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID + 1, &id2))
		return -1;
	if (io_read8(intf, EC_LPC_ADDR_MEMMAP + EC_MEMMAP_HOST_CMD_FLAGS,
		     &flags))
		return -1;

	/* Check for basic support */
	if (id1 != 'E' || id2 != 'C') {
		lprintf(LOG_ERR, "Missing Chromium EC memory map.\n");
		return -1;
	}

	if (flags & EC_HOST_CMD_FLAG_VERSION_3) {
		lprintf(LOG_DEBUG, "Chromium EC LPC command version 3.\n");
		priv->raw = &cros_ec_command_lpc_v3;
		return 3;
	} else if (flags & EC_HOST_CMD_FLAG_LPC_ARGS_SUPPORTED) {
		lprintf(LOG_DEBUG, "Chromium EC LPC command version 1.\n");
		priv->raw = &cros_ec_command_lpc_v1;
		return 2;
	} else {
		lprintf(LOG_DEBUG, "Chromium EC LPC command version 0.\n");
		priv->raw = &cros_ec_command_lpc_v0;
		return 1;
	}
}

/* Sends a command to the EC.  Returns the command status code, or
 * -1 if other error. */
static int cros_ec_command_lpc(struct platform_intf *intf,
			   struct ec_cb *ec,
			   int command, int command_version,
			   const void *indata, int insize,
			   const void *outdata, int outsize)
{
	struct cros_ec_priv *priv;
	int rc = -1;

	if (!intf->cb || !ec || !ec->priv)
		return -1;
	priv = intf->cb->ec->priv;

	if (insize > EC_HOST_PARAM_SIZE || outsize > EC_HOST_PARAM_SIZE) {
		lprintf(LOG_DEBUG, "%s: data size too big\n", __func__);
		return -1;
	}

	lprintf(LOG_DEBUG, "Acquiring CrOS EC lock (timeout=%d sec)...\n",
		CROS_EC_LOCK_TIMEOUT_SECS);
	if (acquire_cros_ec_lock(CROS_EC_LOCK_TIMEOUT_SECS) < 0) {
		lprintf(LOG_ERR, "Could not acquire CrOS EC lock.\n");
		return -1;
	}

	if (cros_ec_command_lpc_detect(intf) < 0) {
		lprintf(LOG_ERR, "Unable to identify LPC "
			"protocol version.\n");
		return -1;
	}

	if (priv && priv->raw)
		rc = priv->raw(intf, ec, command, command_version,
			       indata, insize, outdata, outsize);

	release_cros_ec_lock();
	return rc;
}

struct cros_ec_priv cros_ec_priv_lpc = {
	.cmd		= &cros_ec_command_lpc,
	.addr.io	= EC_LPC_ADDR_HOST_CMD,
	.device_index	= 0,
};

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_ec_probe_lpc(struct platform_intf *intf)
{
	int ret = -1;

	lprintf(LOG_DEBUG, "%s: probing for CrOS EC on LPC...\n", __func__);

	intf->cb->ec->priv = &cros_ec_priv_lpc;
	ret = cros_ec_detect(intf, intf->cb->ec);
	if (ret == 1) {
		lprintf(LOG_DEBUG, "CrOS EC detected on LPC bus\n");
	}

	return ret;
}

struct cros_ec_priv cros_pd_priv_lpc = {
	.cmd            = &cros_ec_command_lpc,
	.addr.io        = EC_LPC_ADDR_HOST_CMD,
	.device_index   = 1,
};

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_pd_probe_lpc(struct platform_intf *intf)
{
	int ret = -1;

	lprintf(LOG_DEBUG, "%s: probing for CrOS PD on LPC...\n", __func__);

	intf->cb->pd->priv = &cros_pd_priv_lpc;
	ret = cros_ec_detect(intf, intf->cb->pd);
	if (ret == 1) {
		lprintf(LOG_DEBUG, "CrOS PD detected on LPC bus\n");
	}

	return ret;
}
