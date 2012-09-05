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
 * gec_cb.c: EC accessor functions / callbacks for use in platform_intf
 */

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/gec.h"
#include "drivers/google/gec_ec_commands.h"

static const char *gec_name(struct platform_intf *intf)
{
	static const char *name = NULL;
	struct ec_response_get_chip_info chip_info;

	if (name)
		return name;

	if (gec_chip_info(intf, &chip_info))
		return NULL;

	name = mosys_strdup(chip_info.name);
	add_destroy_callback(free, (void *)name);
	return name;
}

static const char *gec_vendor(struct platform_intf *intf)
{
	static const char *vendor = NULL;
	struct ec_response_get_chip_info chip_info;

	if (vendor)
		return vendor;

	if (gec_chip_info(intf, &chip_info))
		return NULL;

	vendor = mosys_strdup(chip_info.vendor);
	add_destroy_callback(free, (void *)vendor);
	return vendor;
}

static const char *gec_fw_version(struct platform_intf *intf)
{
	static const char *version = NULL;

	if (version)
		return version;

	version = gec_version(intf);
	if (version)
		add_destroy_callback(free, (void *)version);
	return version;
}

struct ec_cb gec_cb = {
	.vendor		= gec_vendor,
	.name		= gec_name,
	.fw_version	= gec_fw_version,
};
