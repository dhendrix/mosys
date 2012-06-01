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
 * gec_lpc.c: Subset of Google LPC EC interface functionality (ported from
 * chromium os repo)
 */

#include <inttypes.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/gec.h"

#include "intf/io.h"

#include "lib/math.h"

/* I/O addresses for LPC commands */
#define GEC_ADDR_KERNEL_DATA   0x62
#define GEC_ADDR_KERNEL_CMD    0x66
#define GEC_ADDR_KERNEL_PARAM 0x800
#define GEC_ADDR_USER_DATA    0x200
#define GEC_ADDR_USER_CMD     0x204
#define GEC_ADDR_USER_PARAM   0x880

/* LPC command status byte masks */
/* EC has written a byte in the data register and host hasn't read it yet */
#define GEC_STATUS_TO_HOST     0x01
/* Host has written a command/data byte and the EC hasn't read it yet */
#define GEC_STATUS_FROM_HOST   0x02
/* EC is processing a command */
#define GEC_STATUS_PROCESSING  0x04
/* Last write to EC was a command, not data */
#define GEC_STATUS_LAST_CMD    0x08
/* EC is in burst mode.  Chrome EC doesn't support this, so this bit is never
 * set. */
#define GEC_STATUS_BURST_MODE  0x10
/* SCI event is pending (requesting SCI query) */
#define GEC_STATUS_SCI_PENDING 0x20
/* SMI event is pending (requesting SMI query) */
#define GEC_STATUS_SMI_PENDING 0x40
/* (reserved) */
#define GEC_STATUS_RESERVED    0x80

/* EC is busy.  This covers both the EC processing a command, and the host has
 * written a new command but the EC hasn't picked it up yet. */
#define GEC_STATUS_BUSY_MASK \
	(GEC_STATUS_FROM_HOST | GEC_STATUS_PROCESSING)

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

		if (!(data & GEC_STATUS_BUSY_MASK))
			return 0;
	}

	return -1;  /* Timeout */
}

/* Sends a command to the EC.  Returns the command status code, or
 * -1 if other error. */
static int gec_command_lpc(struct platform_intf *intf, int command,
			   const void *indata, int insize,
			   const void *outdata, int outsize)
{
	uint8_t *d;
	int i;
	uint8_t ec_response;

	if (insize > GEC_PARAM_SIZE || outsize > GEC_PARAM_SIZE) {
		lprintf(LOG_DEBUG, "%s: data size too big\n", __func__);
		return -1;
	}

	if (wait_for_ec(intf, GEC_ADDR_USER_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC ready\n",
		                   __func__);
		return -1;
	}

	/* Write data, if any */
	for (i = 0, d = (uint8_t *)indata; i < insize; i++, d++) {
		if (io_write8(intf, GEC_ADDR_USER_PARAM + i, *d))
			return -1;
	}

	if (io_write8(intf, GEC_ADDR_USER_CMD, command))
		return -1;

	if (wait_for_ec(intf, GEC_ADDR_USER_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC response\n",
		                   __func__);
		return -1;
	}

	/* Check result */
	if (io_read8(intf, GEC_ADDR_USER_DATA, &ec_response))
		return -1;

	if (ec_response) {
		lprintf(LOG_DEBUG, "%s: EC returned error result code %d\n",
		                   __func__, ec_response);
		return ec_response;
	}

	/* Read data, if any */
	for (i = 0, d = (uint8_t *)outdata; i < outsize; i++, d++) {
		if (io_read8(intf, GEC_ADDR_USER_PARAM + i, d))
			return -1;
	}

	return 0;
}

struct gec_priv gec_priv_lpc = {
	.cmd		= &gec_command_lpc,
	.addr.io	= GEC_ADDR_USER_CMD,
};

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int gec_probe_lpc(struct platform_intf *intf)
{
	int ret = -1;

	lprintf(LOG_DEBUG, "%s: probing for GEC on LPC...\n", __func__);

	intf->cb->ec->priv = &gec_priv_lpc;
	ret = gec_detect(intf);
	if (ret == 1) {
		lprintf(LOG_DEBUG, "GEC detected on LPC bus\n");
	}

	return ret;
}
