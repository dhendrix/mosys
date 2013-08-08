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

#include "peach_pit.h"

enum peach_pit_board_config peach_pit_board_config;

const char *peach_pit_id_list[] = {
	"Google Peach Pit",
	NULL
};

struct platform_cmd *peach_pit_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

#if 0
static const char *hwids[] = {
	NULL
};
#endif

/* TODO: replace this with proper FDT parsing */
#define FDT_MODEL_NODE	"/proc/device-tree/model"
static char *model_from_fdt(void)
{
	int fd;
	static char model[32];
	int len;

	fd = file_open(FDT_MODEL_NODE, FILE_READ);
	if (fd < 0) {
		lperror(LOG_DEBUG, "Unable to open %s", FDT_MODEL_NODE);
		return NULL;
	}

	memset(model, 0, sizeof(model));
	len = read(fd, &model, sizeof(model));
	if (len < 0) {
		lprintf(LOG_DEBUG, "%s: Could not read FDT\n", __func__);
		return NULL;
	}

	return model;
}

int peach_pit_probe(struct platform_intf *intf)
{
	const char **id;
	char *model = NULL;
	int found = 0;

	model = model_from_fdt();
	if (!model)
		return -1;

	for (id = peach_pit_id_list; id && *id; id++) {
		lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ", model, *id);
		/*
		 * Allow for partial match since FDT-provided model may
		 * include extra revision info at the end.
		 */
		if (!strncmp(*id, model, strlen(*id))) {
			lprintf(LOG_DEBUG, "yes\n");
			found = 1;
			break;
		} else {
			lprintf(LOG_DEBUG, "no\n");
		}
	}

	return found;
}

struct {
	enum mvl3 v3, v2, v1, v0;
	enum peach_pit_board_config config;
} peach_pit_id_map[] = {
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

static int board_config(struct platform_intf *intf)
{
	int i;
	struct gpio_map *rev0, *rev1, *rev2, *rev3;
	enum mvl3 v0, v1, v2, v3;
	enum peach_pit_board_config config = PEACH_PIT_CONFIG_UNKNOWN;

	rev3 = intf->cb->gpio->map(intf, PEACH_PIT_BOARD_REV3);
	rev2 = intf->cb->gpio->map(intf, PEACH_PIT_BOARD_REV2);
	rev1 = intf->cb->gpio->map(intf, PEACH_PIT_BOARD_REV1);
	rev0 = intf->cb->gpio->map(intf, PEACH_PIT_BOARD_REV0);
	if (!rev3 || !rev2 || !rev1 || !rev0) {
		lprintf(LOG_DEBUG, "%s: Unable to determine board "
				"revision\n", __func__);
		return PEACH_PIT_CONFIG_UNKNOWN;
	}

	v3 = exynos5420_read_gpio_mvl(intf, rev3);
	v2 = exynos5420_read_gpio_mvl(intf, rev2);
	v1 = exynos5420_read_gpio_mvl(intf, rev1);
	v0 = exynos5420_read_gpio_mvl(intf, rev0);
	lprintf(LOG_DEBUG, "%s: v3: %u, v2: %u, v1: %u, v0: %u\n",
			__func__, v3, v2, v1, v0);

	for (i = 0; i < ARRAY_SIZE(peach_pit_id_map); i++) {
		if ((v3 == peach_pit_id_map[i].v3) &&
		    (v2 == peach_pit_id_map[i].v2) &&
		    (v1 == peach_pit_id_map[i].v1) &&
		    (v0 == peach_pit_id_map[i].v0)) {
			config = peach_pit_id_map[i].config;
			break;
		}
	}

	return config;
}

static int peach_pit_setup_post(struct platform_intf *intf)
{
	peach_pit_board_config = board_config(intf);
	if (peach_pit_board_config == PEACH_PIT_CONFIG_UNKNOWN)
		return -1;

	if (peach_pit_ec_setup(intf) <= 0)
		return -1;

	return 0;
}

static int peach_pit_destroy(struct platform_intf *intf)
{
	intf->cb->ec->destroy(intf);
	return 0;
}

struct eventlog_cb peach_pit_eventlog_cb = {
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

struct platform_cb peach_pit_cb = {
	.ec 		= &gec_cb,
	.eeprom 	= &peach_pit_eeprom_cb,
	.gpio		= &peach_pit_gpio_cb,
	.memory		= &peach_pit_memory_cb,
	.nvram		= &gec_nvram_cb,
	.sys 		= &peach_pit_sys_cb,
	.eventlog	= &peach_pit_eventlog_cb,
};

struct platform_intf platform_peach_pit = {
	.type		= PLATFORM_ARMV7,
	.name		= "Pit",
	.id_list	= peach_pit_id_list,
	.sub		= peach_pit_sub,
	.cb		= &peach_pit_cb,
	.probe		= &peach_pit_probe,
	.setup_post	= &peach_pit_setup_post,
	.destroy	= &peach_pit_destroy,
};
