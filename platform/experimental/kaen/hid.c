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

#include "mosys/callbacks.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/cypress/cypress_apa.h"

#include "lib/string.h"

#include "intf/i2c.h"

#define KAEN_TP_BUS			0x00
#define KAEN_TP_ADDR			0x67

static const char *kaen_tp_name(struct platform_intf *intf)
{
	static char *name = NULL;

	if (cyapa_get_product_id(intf, KAEN_TP_BUS,
	                         KAEN_TP_ADDR, &name) < 0) {
		free(name);
		return "unknown";
	}

	add_destroy_callback(free, name);
	return name;
}

static const char *kaen_tp_vendor(struct platform_intf *intf)
{
	return "Cypress";
}

static const char *kaen_tp_fw_version(struct platform_intf *intf)
{
	static char *version = NULL;

	if (cyapa_get_firmware_version(intf, KAEN_TP_BUS,
	                               KAEN_TP_ADDR, &version) < 0) {
		free(version);
		return "unknown";
	}

	add_destroy_callback(free, version);
	return version;
}

static const char *kaen_tp_hw_version(struct platform_intf *intf)
{
	static char *version = NULL;

	if (cyapa_get_hardware_version(intf, KAEN_TP_BUS,
	                               KAEN_TP_ADDR, &version) < 0) {
		free(version);
		return "unknown";
	}

	add_destroy_callback(free, version);
	return version;
}

static struct hid_tp_cb kaen_tp_cb = {
	.vendor		= kaen_tp_vendor,
	.name		= kaen_tp_name,
	.fw_version	= kaen_tp_fw_version,
	.hw_version	= kaen_tp_hw_version,
};

struct hid_cb kaen_hid_cb = {
	.tp		= &kaen_tp_cb,
};
