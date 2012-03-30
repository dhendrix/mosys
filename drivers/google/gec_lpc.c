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

/* Sends a command to the EC.  Returns the command status code, or
 * -1 if other error. */
static int ec_command(struct platform_intf *intf, int command,
                      const void *indata, int insize, void *outdata,
                      int outsize) {
	uint8_t *d;
	int i;
	uint8_t ec_response;

	if (insize > EC_LPC_PARAM_SIZE || outsize > EC_LPC_PARAM_SIZE) {
		lprintf(LOG_DEBUG, "%s: data size too big\n", __func__);
		return -1;
	}

	if (wait_for_ec(intf, EC_LPC_ADDR_USER_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC ready\n",
		                   __func__);
		return -1;
	}

	/* Write data, if any */
	for (i = 0, d = (uint8_t *)indata; i < insize; i++, d++) {
		if (io_write8(intf, EC_LPC_ADDR_USER_PARAM + i, *d))
			return -1;
	}

	if (io_write8(intf, EC_LPC_ADDR_USER_CMD, command))
		return -1;

	if (wait_for_ec(intf, EC_LPC_ADDR_USER_CMD, 1000000)) {
		lprintf(LOG_DEBUG, "%s: timeout waiting for EC response\n",
		                   __func__);
		return -1;
	}

	/* Check result */
	if (io_read8(intf, EC_LPC_ADDR_USER_DATA, &ec_response))
		return -1;

	if (ec_response) {
		lprintf(LOG_DEBUG, "%s: EC returned error result code %d\n",
		                   __func__, ec_response);
		return ec_response;
	}

	/* Read data, if any */
	for (i = 0, d = (uint8_t *)outdata; i < outsize; i++, d++) {
		if (io_read8(intf, EC_LPC_ADDR_USER_PARAM + i, d))
			return -1;
	}

	return 0;
}

int gec_hello(struct platform_intf *intf)
{
	struct lpc_params_hello p;
	struct lpc_response_hello r;
	int rv;

	p.in_data = 0xa0b0c0d0;

	rv = ec_command(intf, EC_LPC_COMMAND_HELLO, &p,
	                sizeof(p), &r, sizeof(r));
	if (rv)
		return rv;

	if (r.out_data != 0xa1b2c3d4) {
		lprintf(LOG_ERR, "Expected response 0x%08x, got 0x%08x\n",
			0xa1b2c3d4, r.out_data);
		rv = -1;
	}

	lprintf(LOG_DEBUG, "%s: EC says hello!\n", __func__);
	return rv;
}

const char *gec_version(struct platform_intf *intf)
{
	static const char *const fw_copies[] = {"unknown", "RO", "A", "B"};
	struct lpc_response_get_version r;
	struct lpc_response_get_build_info r2;
	const char *ret = NULL;

	if (ec_command(intf, EC_LPC_COMMAND_GET_VERSION,
	               NULL, 0, &r, sizeof(r)))
		return NULL;

	if (ec_command(intf, EC_LPC_COMMAND_GET_BUILD_INFO,
			NULL, 0, &r2, sizeof(r2)))
		return NULL;

	/* Ensure versions are null-terminated before we print them */
	r.version_string_ro[sizeof(r.version_string_ro) - 1] = '\0';
	r.version_string_rw_a[sizeof(r.version_string_rw_a) - 1] = '\0';
	r.version_string_rw_b[sizeof(r.version_string_rw_b) - 1] = '\0';
	r2.build_string[sizeof(r2.build_string) - 1] = '\0';

	/* Print versions */
	lprintf(LOG_DEBUG, "RO version:    %s\n", r.version_string_ro);
	lprintf(LOG_DEBUG, "RW-A version:  %s\n", r.version_string_rw_a);
	lprintf(LOG_DEBUG, "RW-B version:  %s\n", r.version_string_rw_b);
	lprintf(LOG_DEBUG, "Firmware copy: %s\n",
	       (r.current_image < ARRAY_SIZE(fw_copies) ?
		fw_copies[r.current_image] : "?"));
	lprintf(LOG_DEBUG, "Build info:    %s\n", r2.build_string);

	switch (r.current_image) {
	case EC_LPC_IMAGE_RO:
		ret = mosys_strdup(r.version_string_ro);
		break;
	case EC_LPC_IMAGE_RW_A:
		ret = mosys_strdup(r.version_string_rw_a);
		break;
	case EC_LPC_IMAGE_RW_B:
		ret = mosys_strdup(r.version_string_rw_b);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: cannot determine version\n", __func__);
		break;
	}

	return ret;
}
