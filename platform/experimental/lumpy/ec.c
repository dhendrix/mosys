/*
 * Copyright 2011, Google Inc.
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

static const char *lumpy_ec_name(struct platform_intf *intf)
{
	return mec1308_sio_name(intf);
}

static const char *lumpy_ec_vendor(struct platform_intf *intf)
{
	return mec1308_sio_vendor(intf);
}

static const char *lumpy_ec_fw_version(struct platform_intf *intf)
{
	static uint8_t version[MEC1308_MBX_DATA_LEN];

	memset(version, 0, sizeof(version));

	if (mec1308_mbx_fw_version(intf, version, MEC1308_MBX_DATA_LEN)){
		lprintf(LOG_DEBUG, "%s: Unable to obtain firmware version\n",
		                   __func__);
		return NULL;
	}

	return version;
}

struct ec_cb lumpy_ec_cb = {
	.vendor		= lumpy_ec_vendor,
	.name		= lumpy_ec_name,
	.fw_version	= lumpy_ec_fw_version,
};

int lumpy_ec_setup(struct platform_intf *intf)
{
	/* invert logic -- mec1308_detect will return 1 if it finds an EC */
	if (!mec1308_detect(intf))
		return -1;

	if (mec1308_mbx_setup(intf) < 0)
		return -1;

	return 0;
}

void lumpy_ec_destroy(struct platform_intf *intf)
{
	mec1308_mbx_teardown(intf);
}
