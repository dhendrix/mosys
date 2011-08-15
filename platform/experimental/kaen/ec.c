/*
 * Copyright (C) 2011 Google Inc.
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
 * GOOGLE OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF GOOGLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
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

	if (intf->op->i2c->read_reg(intf, 2, 0x1b, 0xf1, 2, buf) != 2) {
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
