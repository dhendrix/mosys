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
	UNKNOWN = -1,
	BRAIN,
	DANGER,
	EMILE,
	FIEVEL,
	GUS,
	JAQ,
	JERRY,
	MICKEY,
	MIGHTY,
        MINNIE,
	PINKY,
	REMY,
	RIALTO,
	SPEEDY,
	THEA,
	TIGER
};

static enum pinky_boards probed_board = UNKNOWN;

struct veyron_probe_id {
	const char *name;
	const char *fdt_compat;
	int has_ec;
} veyron_id_list[] = {
	[BRAIN]		= { "Brain", "google,veyron-brain", 0 },
	[DANGER]	= { "Danger", "google,veyron-danger", 0 },
	[EMILE]		= { "Emile", "google,veyron-emile", 0 },
	[FIEVEL]	= { "Fievel", "google,veyron-fievel", 0 },
	[GUS]		= { "Gus", "google,veyron-gus", 1 },
	[JAQ]		= { "Jaq", "google,veyron-jaq", 1 },
	[JERRY]		= { "Jerry", "google,veyron-jerry", 1 },
	[MICKEY]	= { "Mickey", "google,veyron-mickey", 0 },
	[MIGHTY]	= { "Mighty", "google,veyron-mighty", 1 },
        [MINNIE]	= { "Minnie", "google,veyron-minnie", 1 },
	[PINKY]		= { "Pinky", "google,veyron-pinky", 1 },
	[REMY]		= { "Remy", "google,veyron-remy", 1 },
	[RIALTO]	= { "Rialto", "google,veyron-rialto", 0 },
	[SPEEDY]	= { "Speedy", "google,veyron-speedy", 1 },
	[THEA]		= { "Thea", "google,veyron-thea", 1 },
	[TIGER]		= { "Tiger", "google,veyron-tiger", 0 },
};

#define PINKY_CMD_EC_NUM	0
struct platform_cmd *pinky_sub[] = {
	/* Keep this as the first entry. intf->sub will be set to point to
         * the next entry if it turns out that we don't have an EC. */
	[PINKY_CMD_EC_NUM] = &cmd_ec,

	&cmd_eeprom,
//	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_psu,
	&cmd_eventlog,
	NULL
};

static int pinky_probe(struct platform_intf *intf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(veyron_id_list); i++) {
		const char **compat = &veyron_id_list[i].fdt_compat;

		if (probe_fdt_compatible(compat, 1, 1) == 0) {
			lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT "
				"compatible node.\n", veyron_id_list[i].name);
			intf->name = veyron_id_list[i].name;
			probed_board = i;
			break;
		}
	}

	lprintf(LOG_DEBUG, "%s: probed_board: %d\n", __func__, probed_board);
	return probed_board > UNKNOWN ? 1 : 0;
}

static int pinky_setup_post(struct platform_intf *intf)
{
	if (veyron_id_list[probed_board].has_ec) {
		if (cros_ec_setup(intf) < 0)
			return -1;
		intf->cb->nvram = &cros_ec_nvram_cb;
	} else {
		intf->cb->ec = NULL;
		intf->cb->nvram = &cros_spi_flash_nvram_cb;
		intf->sub = &pinky_sub[PINKY_CMD_EC_NUM + 1];
	}

	pinky_eeprom_setup(intf, veyron_id_list[probed_board].has_ec);

	return 0;
}

static int pinky_destroy(struct platform_intf *intf)
{
	if (intf->cb->ec && intf->cb->ec->destroy)
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
	.psu 		= &pinky_psu_cb,
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
