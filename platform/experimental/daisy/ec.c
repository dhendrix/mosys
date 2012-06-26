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

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/gec.h"

#include "lib/string.h"

#include "intf/i2c.h"

#define	DAISY_EC_ADDRESS 0x1e

static const char *daisy_ec_name(struct platform_intf *intf)
{
	static const char *name = NULL;
	struct gec_response_get_chip_info chip_info;

	if (name)
		return name;

	if (gec_chip_info(intf, &chip_info))
		return NULL;

	name = mosys_strdup(chip_info.name);
	add_destroy_callback(free, (void *)name);
	return name;
}

static const char *daisy_ec_vendor(struct platform_intf *intf)
{
	static const char *vendor = NULL;
	struct gec_response_get_chip_info chip_info;

	if (vendor)
		return vendor;

	if (gec_chip_info(intf, &chip_info))
		return NULL;

	vendor = mosys_strdup(chip_info.vendor);
	add_destroy_callback(free, (void *)vendor);
	return vendor;
}

static const char *daisy_ec_fw_version(struct platform_intf *intf)
{
	static const char *version = NULL;

	if (version)
		return version;

	version = gec_version(intf);
	if (version)
		add_destroy_callback(free, (void *)version);
	return version;
}

struct gec_priv daisy_ec_priv = {
	.addr.i2c.addr	= DAISY_EC_ADDRESS,
};

struct ec_cb daisy_ec_cb = {
	.vendor		= daisy_ec_vendor,
	.name		= daisy_ec_name,
	.fw_version	= daisy_ec_fw_version,
	.priv		= &daisy_ec_priv,
};

int daisy_ec_setup(struct platform_intf *intf)
{
	int ret;

	ret = gec_probe_i2c(intf);
	if (ret == 1)
		lprintf(LOG_DEBUG, "GEC found on I2C bus\n");
	else if (ret == 0)
		lprintf(LOG_DEBUG, "GEC not found on I2C bus\n");
	else
		lprintf(LOG_ERR, "Error when probing GEC on I2C bus\n");

	return ret;
}