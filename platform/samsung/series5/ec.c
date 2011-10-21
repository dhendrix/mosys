/*
 * Copyright (C) 2010 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Google Inc. or the names of contributors or
 * licensors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * GOOGLE INC AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation. 
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
