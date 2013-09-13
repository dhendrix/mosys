/*
 * Copyright 2013, Google Inc.
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

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/google/gec.h"
#include "drivers/samsung/exynos5420/gpio.h"

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "peach.h"

enum peach_board_config peach_board_config;

struct probe_id {
	const char *probe_name;
	const char *canonical_name;
};

char *peach_id_list_old[] = {
	/* old style list for certain rarely-used commands (-S, -p) */
	"Google Peach Pit",
	"Google Peach Kirby",
	NULL,
};

struct probe_id peach_id_list[] = {
	{ "Google Peach Pit", "Pit" },
	{ "Google Peach Kirby", "Kirby" },
	{ NULL, NULL },
};

struct platform_cmd *peach_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

int peach_probe(struct platform_intf *intf)
{
	char *model = NULL;
	int found = 0;
	struct probe_id *id;

	model = fdt_model();
	if (!model)
		return -1;

	for (id = &peach_id_list[0]; id && id->probe_name; id++) {
		lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ",
					model, id->probe_name);
		/*
		 * Allow for partial match since FDT-provided model may
		 * include extra revision info at the end.
		 */
		if (!strncmp(id->probe_name, model, strlen(id->probe_name))) {
			lprintf(LOG_DEBUG, "yes\n");
			found = 1;
			break;
		} else {
			lprintf(LOG_DEBUG, "no\n");
		}
	}

	intf->name = id->canonical_name;
	return found;
}

struct id_map {
	enum mvl3 v3, v2, v1, v0;
	enum peach_board_config config;
};

struct id_map peach_pit_id_map[] = {
	/*  REV3     REV3     REV1     REV0       config */
	{ LOGIC_0, LOGIC_0, LOGIC_0, LOGIC_0, PEACH_PIT_CONFIG_PROTO },

	/* For EVT board IDs, see chrome-os-partner:19179 (comment #2) */
	{ LOGIC_0, LOGIC_0, LOGIC_1, LOGIC_1, PEACH_PIT_CONFIG_EVT_2GB },
	{ LOGIC_0, LOGIC_0, LOGIC_Z, LOGIC_0, PEACH_PIT_CONFIG_EVT_4GB },

	{ LOGIC_0, LOGIC_0, LOGIC_Z, LOGIC_Z, PEACH_PIT_CONFIG_DVT1_2GB },
	{ LOGIC_0, LOGIC_1, LOGIC_0, LOGIC_1, PEACH_PIT_CONFIG_DVT1_4GB },

	{ LOGIC_0, LOGIC_1, LOGIC_1, LOGIC_0, PEACH_PIT_CONFIG_DVT2_2GB },
	{ LOGIC_0, LOGIC_1, LOGIC_1, LOGIC_Z, PEACH_PIT_CONFIG_DVT2_4GB },

	{ LOGIC_0, LOGIC_1, LOGIC_Z, LOGIC_1, PEACH_PIT_CONFIG_PVT1_2GB },
	{ LOGIC_Z, LOGIC_0, LOGIC_Z, LOGIC_0, PEACH_PIT_CONFIG_PVT1_4GB },

	{ LOGIC_0, LOGIC_Z, LOGIC_0, LOGIC_Z, PEACH_PIT_CONFIG_PVT2_2GB },
	{ LOGIC_0, LOGIC_Z, LOGIC_1, LOGIC_1, PEACH_PIT_CONFIG_PVT2_4GB },

	{ LOGIC_0, LOGIC_Z, LOGIC_Z, LOGIC_0, PEACH_PIT_CONFIG_MP_2GB },
	{ LOGIC_0, LOGIC_Z, LOGIC_Z, LOGIC_Z, PEACH_PIT_CONFIG_MP_4GB },
};

struct id_map peach_kirby_id_map[] = {
	/*  REV3     REV2     REV1     REV0       config */
	{ LOGIC_0, LOGIC_0, LOGIC_0, LOGIC_0, PEACH_KIRBY_CONFIG_PROTO0 },
	{ LOGIC_0, LOGIC_0, LOGIC_0, LOGIC_1, PEACH_KIRBY_CONFIG_PROTO1 },
	{ LOGIC_0, LOGIC_0, LOGIC_0, LOGIC_Z, PEACH_KIRBY_CONFIG_EVT },
	{ LOGIC_0, LOGIC_0, LOGIC_1, LOGIC_0, PEACH_KIRBY_CONFIG_DVT },
	{ LOGIC_0, LOGIC_0, LOGIC_1, LOGIC_1, PEACH_KIRBY_CONFIG_PVT },
	{ LOGIC_0, LOGIC_0, LOGIC_1, LOGIC_Z, PEACH_KIRBY_CONFIG_MP },

	/* Strappings set aside, but not assigned a particular ID */
	{ LOGIC_0, LOGIC_0, LOGIC_Z, LOGIC_0, PEACH_KIRBY_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_0, LOGIC_Z, LOGIC_1, PEACH_KIRBY_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_0, LOGIC_Z, LOGIC_Z, PEACH_KIRBY_CONFIG_RSVD },
};

static int get_peach_board_config(struct platform_intf *intf)
{
	int i, count;
	struct gpio_map *rev0, *rev1, *rev2, *rev3;
	enum mvl3 v0, v1, v2, v3;
	enum peach_board_config config = PEACH_CONFIG_UNKNOWN;
	struct id_map *map;
	char logic[] = {
		[LOGIC_Z] = 'Z',
		[LOGIC_0] = '0',
		[LOGIC_1] = '1',
	};

	if (!strcmp(intf->name, "Pit")) {
		map = peach_pit_id_map;
		count = ARRAY_SIZE(peach_pit_id_map);
	} else if (!strcmp(intf->name, "Kirby")) {
		map = peach_kirby_id_map;
		count = ARRAY_SIZE(peach_kirby_id_map);
	} else {
		return PEACH_CONFIG_UNKNOWN;
	}

	rev3 = intf->cb->gpio->map(intf, PEACH_BOARD_REV3);
	rev2 = intf->cb->gpio->map(intf, PEACH_BOARD_REV2);
	rev1 = intf->cb->gpio->map(intf, PEACH_BOARD_REV1);
	rev0 = intf->cb->gpio->map(intf, PEACH_BOARD_REV0);
	if (!rev3 || !rev2 || !rev1 || !rev0) {
		lprintf(LOG_DEBUG, "%s: Unable to determine board "
				"revision\n", __func__);
		return PEACH_CONFIG_UNKNOWN;
	}

	v3 = exynos5420_read_gpio_mvl(intf, rev3);
	v2 = exynos5420_read_gpio_mvl(intf, rev2);
	v1 = exynos5420_read_gpio_mvl(intf, rev1);
	v0 = exynos5420_read_gpio_mvl(intf, rev0);
	lprintf(LOG_DEBUG, "%s: v3: %c, v2: %c, v1: %c, v0: %c\n",
			__func__, logic[v3], logic[v2], logic[v1], logic[v0]);

	for (i = 0; i < count; i++) {
		if ((v3 == map[i].v3) &&
		    (v2 == map[i].v2) &&
		    (v1 == map[i].v1) &&
		    (v0 == map[i].v0)) {
			config = map[i].config;
			break;
		}
	}

	return config;
}

static int peach_setup_post(struct platform_intf *intf)
{
	peach_board_config = get_peach_board_config(intf);
	if (peach_board_config == PEACH_CONFIG_UNKNOWN)
		return -1;

	if (peach_ec_setup(intf) <= 0)
		return -1;

	return 0;
}

static int peach_destroy(struct platform_intf *intf)
{
	intf->cb->ec->destroy(intf);
	return 0;
}

struct eventlog_cb peach_eventlog_cb = {
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

struct platform_cb peach_cb = {
	.ec 		= &gec_cb,
	.eeprom 	= &peach_eeprom_cb,
	.gpio		= &peach_gpio_cb,
	.memory		= &peach_memory_cb,
	.nvram		= &gec_nvram_cb,
	.sys 		= &peach_sys_cb,
	.eventlog	= &peach_eventlog_cb,
};

struct platform_intf platform_peach = {
	.type		= PLATFORM_ARMV7,
	.name		= "Peach",
	.id_list	= peach_id_list_old,
	.sub		= peach_sub,
	.cb		= &peach_cb,
	.probe		= &peach_probe,
	.setup_post	= &peach_setup_post,
	.destroy	= &peach_destroy,
};
