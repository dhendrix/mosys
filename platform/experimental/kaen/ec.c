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

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/string.h"

#include "intf/i2c.h"

static const char *kaen_ec_name(struct platform_intf *intf)
{
	return "npce783";
}

static const char *kaen_ec_vendor(struct platform_intf *intf)
{
	return "nuvoton";
}

static const char *kaen_ec_fw_version(struct platform_intf *intf)
{
	static char *version = NULL;
	uint8_t buf[2];

	if (version)
		return version;

	if (intf->op->i2c->smbus_read_reg(intf, 2, 0x1b, 0xf1, 2, buf) != 2) {
		lprintf(LOG_ERR, "%s: failed to read EC firmware "
		        "version\n", __func__);
		return NULL;
	}

	version = buf2str(buf, sizeof(buf));
	return version;
}

struct ec_cb kaen_ec_cb = {
	.vendor		= kaen_ec_vendor,
	.name		= kaen_ec_name,
	.fw_version	= kaen_ec_fw_version,
};
