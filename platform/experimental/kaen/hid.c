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
