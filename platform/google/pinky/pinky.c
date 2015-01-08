/*
 * Copyright 2014, Google Inc.
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

#include <stdlib.h>
#include <unistd.h>

#include "pinky.h"
#include "drivers/google/cros_ec.h"
#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

enum pinky_boards {
	GUS,
	JAQ,
	JERRY,
	MIGHTY,
	PINKY,
	SPEEDY
};

const char *pinky_id_list[] = {
	[GUS] = "google,veyron-gus",
	[JAQ] = "google,veyron-jaq",
	[JERRY] = "google,veyron-jerry",
	[MIGHTY] = "google,veyron-mighty",
	[PINKY] = "google,veyron-pinky",
	[SPEEDY] = "google,veyron-speedy",
	NULL,
};

struct platform_cmd *pinky_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
//	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

int pinky_probe(struct platform_intf *intf)
{
	int index;

	index = probe_fdt_compatible(&pinky_id_list[0],
					ARRAY_SIZE(pinky_id_list), 1);
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", pinky_id_list[index]);

		if (index == GUS)
			intf->name = "Gus";
		if (index == JAQ)
			intf->name = "Jaq";
		if (index == JERRY)
			intf->name = "Jerry";
		if (index == MIGHTY)
			intf->name = "Mighty";
		if (index == SPEEDY)
			intf->name = "Speedy";
	}

	return index >= 0 ? 1 : 0;
}

static int pinky_setup_post(struct platform_intf *intf)
{
	if (pinky_ec_setup(intf) <= 0)
		return -1;
	return 0;
}

static int pinky_destroy(struct platform_intf *intf)
{
	if (intf->cb->ec->destroy)
		intf->cb->ec->destroy(intf, intf->cb->ec);
	return 0;
}

struct eventlog_cb pinky_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
	.add		= &elog_add_event_manually,
	.clear		= &elog_clear_manually,
	.fetch		= &elog_fetch_from_flash,
	.write		= &elog_write_to_flash,
};

struct platform_cb pinky_cb = {
	.ec		= &cros_ec_cb,
	.eeprom 	= &pinky_eeprom_cb,
//	.gpio		= &pinky_gpio_cb,
	.memory		= &pinky_memory_cb,
	.nvram		= &cros_ec_nvram_cb,
	.sys 		= &pinky_sys_cb,
	.eventlog	= &pinky_eventlog_cb,
};

struct platform_intf platform_pinky = {
	.type		= PLATFORM_ARMV7,
	.name		= "Pinky",
	.sub		= pinky_sub,
	.cb		= &pinky_cb,
	.probe		= &pinky_probe,
	.setup_post	= &pinky_setup_post,
	.destroy	= &pinky_destroy,
};
