/*
 * Copyright 2012, Google Inc.
 * All rights reserved.
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
 */

#include <ctype.h>
#include <inttypes.h>

#include "drivers/smsc/mec1308.h"

#include "mosys/log.h"
#include "mosys/platform.h"

static const char *samsung_series5_ec_name(struct platform_intf *intf)
{
	return mec1308_sio_name(intf);
}

static const char *samsung_series5_ec_vendor(struct platform_intf *intf)
{
	return mec1308_sio_vendor(intf);
}

static const char *samsung_series5_ec_fw_version(struct platform_intf *intf)
{
	static uint8_t version[MEC1308_MBX_DATA_LEN];
	int i, num_tries = 3;

	memset(version, 0, sizeof(version));

	for (i = 0; i < num_tries; i++) {
		int j, retry = 0;

		if (mec1308_mbx_fw_version(intf,
		                           version,
		                           MEC1308_MBX_DATA_LEN)) {
			lprintf(LOG_DEBUG, "%s: unable to issue command, "
			                   "attempting to unwedge EC\n",
			                   __func__);
			mec1308_mbx_exit_passthru_mode(intf);
		}

		for (j = 0; j < MEC1308_MBX_DATA_LEN; j++) {
			if (!isascii(version[j])) {
				lprintf(LOG_DEBUG, "%s: bad output detected: \"%s\", "
				                   "attempting to unwedge EC\n",
						   __func__, version);
				mec1308_mbx_exit_passthru_mode(intf);
			}
			retry = 1;
		}

		if (retry)
			continue;
		lprintf(LOG_DEBUG, "%s: ec firmware version: \"%s\"\n",
		                   __func__, version);
		break;
	}

	return version;
}

struct ec_cb samsung_series5_ec_cb = {
	.vendor		= samsung_series5_ec_vendor,
	.name		= samsung_series5_ec_name,
	.fw_version	= samsung_series5_ec_fw_version,
};

int samsung_series5_ec_setup(struct platform_intf *intf)
{
	/* invert logic -- mec1308_detect will return 1 if it finds an EC */
	if (!mec1308_detect(intf))
		return -1;

	if (mec1308_mbx_setup(intf) < 0)
		return -1;

	/* Attempt to exit SPI passthru mode. This is a benign operation if
	   we are not already in passthru mode. */
	lprintf(LOG_DEBUG, "%s: attempting to exit passthru mode (this may"
	                   " fail safely)\n", __func__);
	mec1308_mbx_exit_passthru_mode(intf);

	return 0;
}

void samsung_series5_ec_destroy(struct platform_intf *intf)
{
	mec1308_mbx_teardown(intf);
}
