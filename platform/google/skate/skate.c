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
#include "drivers/google/cros_ec.h"
#include "drivers/samsung/exynos5250/gpio.h"

#include "lib/elog.h"
#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "skate.h"

static const char *probed_platform_id;
enum skate_board_config skate_board_config;

const char *skate_id_list[] = {
	"Google Skate",
	NULL
};

struct platform_cmd *skate_sub[] = {
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

int skate_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;
	char *model = NULL;

	if (probed)
		return status;

	model = model_from_fdt();

	for (id = skate_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto skate_probe_exit;
		}

		if (model) {
			lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ", model, *id);
			if (!strcmp(*id, model)) {
				lprintf(LOG_DEBUG, "yes\n");
				status = 1;
				goto skate_probe_exit;
			} else {
				lprintf(LOG_DEBUG, "no\n");
			}
		}
	}

#if 0
	if (probe_hwid(hwids)) {
		status = 1;
		goto skate_probe_exit;
	}
#endif

skate_probe_exit:
	probed = 1;
	return status;
}

struct {
	enum mvl3 v0, v1, v2;
	enum skate_board_config config;
} skate_id_map[] = {
/* 0 = low, 1 = high, 2 = Z */
/* REV0 is LSB, REV 2 is MSB */
		/*  REV0     REV1     REV2       config */
/* rev 0 */	{ LOGIC_0, LOGIC_0, LOGIC_0, SKATE_CONFIG_PROTO_MICRON },
/* rev 1 */	{ LOGIC_0, LOGIC_0, LOGIC_1, SKATE_CONFIG_PROTO_HYNIX },
/* rev 2 */	{ LOGIC_0, LOGIC_0, LOGIC_Z, SKATE_CONFIG_PROTO_ELPIDA },
/* rev 3 */	{ LOGIC_0, LOGIC_1, LOGIC_0, SKATE_CONFIG_EVT_MICRON },
/* rev  4 */	{ LOGIC_0, LOGIC_1, LOGIC_1, SKATE_CONFIG_EVT_HYNIX },
/* rev  5 */	{ LOGIC_0, LOGIC_1, LOGIC_Z, SKATE_CONFIG_EVT_ELPIDA },
/* rev  6 */	{ LOGIC_0, LOGIC_Z, LOGIC_0, SKATE_CONFIG_DVT_MICRON },
/* rev  7 */	{ LOGIC_0, LOGIC_Z, LOGIC_1, SKATE_CONFIG_DVT_HYNIX },
/* rev  8 */	{ LOGIC_0, LOGIC_Z, LOGIC_Z, SKATE_CONFIG_DVT_ELPIDA },
/* rev  9 */	{ LOGIC_1, LOGIC_0, LOGIC_0, SKATE_CONFIG_PVT_MICRON },
/* rev 10 */	{ LOGIC_1, LOGIC_0, LOGIC_1, SKATE_CONFIG_PVT_HYNIX },
/* rev 11 */	{ LOGIC_1, LOGIC_0, LOGIC_Z, SKATE_CONFIG_PVT_ELPIDA },
/* rev 12 */	{ LOGIC_1, LOGIC_1, LOGIC_0, SKATE_CONFIG_MP_MICRON },
/* rev 13 */	{ LOGIC_1, LOGIC_1, LOGIC_1, SKATE_CONFIG_MP_HYNIX },
/* rev 14 */	{ LOGIC_1, LOGIC_1, LOGIC_Z, SKATE_CONFIG_MP_ELPIDA },
/* rev 15 */	{ LOGIC_1, LOGIC_Z, LOGIC_0, SKATE_CONFIG_RSVD },
/* rev 16 */	{ LOGIC_1, LOGIC_Z, LOGIC_1, SKATE_CONFIG_RSVD },
		{ LOGIC_1, LOGIC_Z, LOGIC_Z, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_0, LOGIC_0, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_0, LOGIC_1, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_0, LOGIC_Z, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_1, LOGIC_0, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_1, LOGIC_1, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_1, LOGIC_Z, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_Z, LOGIC_0, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_Z, LOGIC_1, SKATE_CONFIG_RSVD },
		{ LOGIC_Z, LOGIC_Z, LOGIC_Z, SKATE_CONFIG_RSVD },
};

static int skate_get_board_config(struct platform_intf *intf)
{
	int i;
	struct gpio_map *rev0, *rev1, *rev2;
	enum mvl3 v0, v1, v2;
	enum skate_board_config config = SKATE_CONFIG_UNKNOWN;

	rev0 = intf->cb->gpio->map(intf, SKATE_BOARD_REV0);
	rev1 = intf->cb->gpio->map(intf, SKATE_BOARD_REV1);
	rev2 = intf->cb->gpio->map(intf, SKATE_BOARD_REV2);
	if (!rev0 || !rev1 || !rev2) {
		lprintf(LOG_DEBUG, "%s: Unable to determine board "
				"revision\n", __func__);
		return SKATE_CONFIG_UNKNOWN;
	}

	v0 = exynos5250_read_gpio_mvl(intf, rev0);
	v1 = exynos5250_read_gpio_mvl(intf, rev1);
	v2 = exynos5250_read_gpio_mvl(intf, rev2);
	lprintf(LOG_DEBUG, "%s: v0: %u, v1: %u, v2: %u => rev %u\n",
			__func__, v0, v1, v2, v0 + v1 * 3 + v2 * 9);

	for (i = 0; i < ARRAY_SIZE(skate_id_map); i++)
		if ((v0 == skate_id_map[i].v0) &&
		    (v1 == skate_id_map[i].v1) &&
		    (v2 == skate_id_map[i].v2))
			config = skate_id_map[i].config;

	return config;
}

static int skate_setup_post(struct platform_intf *intf)
{
	skate_board_config = skate_get_board_config(intf);
	if (skate_board_config == SKATE_CONFIG_UNKNOWN)
		return -1;

	if (skate_ec_setup(intf) <= 0)
		return -1;

	return 0;
}

static int skate_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	return 0;
}

struct eventlog_cb skate_eventlog_cb = {
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

struct platform_cb skate_cb = {
	.ec 		= &cros_ec_cb,
	.eeprom 	= &skate_eeprom_cb,
	.gpio		= &skate_gpio_cb,
	.memory		= &skate_memory_cb,
	.nvram		= &cros_ec_nvram_cb,
	.sys 		= &skate_sys_cb,
	.eventlog	= &skate_eventlog_cb,
};

struct platform_intf platform_skate = {
	.type		= PLATFORM_ARMV7,
	.name		= "Skate",
	.id_list	= skate_id_list,
	.sub		= skate_sub,
	.cb		= &skate_cb,
	.probe		= &skate_probe,
	.setup_post	= &skate_setup_post,
	.destroy	= &skate_destroy,
};
