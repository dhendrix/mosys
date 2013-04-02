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
#include "drivers/samsung/exynos5.h"

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "spring.h"

static const char *probed_platform_id;
enum spring_board_config spring_board_config;

const char *spring_id_list[] = {
	"Google Spring",
	NULL
};

struct platform_cmd *spring_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
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

int spring_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;
	char *model = NULL;

	if (probed)
		return status;

	model = model_from_fdt();

	for (id = spring_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto spring_probe_exit;
		}

		if (model) {
			lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ", model, *id);
			if (!strcmp(*id, model)) {
				lprintf(LOG_DEBUG, "yes\n");
				status = 1;
				goto spring_probe_exit;
			} else {
				lprintf(LOG_DEBUG, "no\n");
			}
		}
	}

#if 0
	if (probe_hwid(hwids)) {
		status = 1;
		goto spring_probe_exit;
	}
#endif

spring_probe_exit:
	probed = 1;
	return status;
}

/* TODO: implement board ID map when assignments are finalized */
struct {
	enum mvl3 v0, v1, v2;
	enum spring_board_config config;
} spring_id_map[] = {
	/*  REV0     REV1     REV2       config */
	{ LOGIC_0, LOGIC_0, LOGIC_0, SPRING_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_0, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_0, LOGIC_Z, SPRING_CONFIG_PROTO },
	{ LOGIC_0, LOGIC_1, LOGIC_0, SPRING_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_1, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_1, LOGIC_Z, SPRING_CONFIG_DVT_NANYA },
	{ LOGIC_0, LOGIC_Z, LOGIC_0, SPRING_CONFIG_PVT_MICRON },
	{ LOGIC_0, LOGIC_Z, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_0, LOGIC_Z, LOGIC_Z, SPRING_CONFIG_RSVD },
	{ LOGIC_1, LOGIC_0, LOGIC_0, SPRING_CONFIG_RSVD },
	{ LOGIC_1, LOGIC_0, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_1, LOGIC_0, LOGIC_Z, SPRING_CONFIG_EVT_NANYA },
	{ LOGIC_1, LOGIC_1, LOGIC_0, SPRING_CONFIG_RSVD },
	{ LOGIC_1, LOGIC_1, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_1, LOGIC_1, LOGIC_Z, SPRING_CONFIG_DVT_MICRON },
	{ LOGIC_1, LOGIC_Z, LOGIC_0, SPRING_CONFIG_MP_NANYA },
	{ LOGIC_1, LOGIC_Z, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_1, LOGIC_Z, LOGIC_Z, SPRING_CONFIG_RSVD },
	{ LOGIC_Z, LOGIC_0, LOGIC_0, SPRING_CONFIG_MP_MICRON },
	{ LOGIC_Z, LOGIC_0, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_Z, LOGIC_0, LOGIC_Z, SPRING_CONFIG_RSVD },
	{ LOGIC_Z, LOGIC_1, LOGIC_0, SPRING_CONFIG_RSVD },
	{ LOGIC_Z, LOGIC_1, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_Z, LOGIC_1, LOGIC_Z, SPRING_CONFIG_EVT_MICRON },
	{ LOGIC_Z, LOGIC_Z, LOGIC_0, SPRING_CONFIG_PVT_NANYA },
	{ LOGIC_Z, LOGIC_Z, LOGIC_1, SPRING_CONFIG_RSVD },
	{ LOGIC_Z, LOGIC_Z, LOGIC_Z, SPRING_CONFIG_RSVD },
};

static int spring_get_board_config(struct platform_intf *intf)
{
	int i;
	struct gpio_map *rev0, *rev1, *rev2;
	enum mvl3 v0, v1, v2;
	enum spring_board_config config = SPRING_CONFIG_UNKNOWN;

	rev0 = intf->cb->gpio->map(intf, SPRING_BOARD_REV0);
	rev1 = intf->cb->gpio->map(intf, SPRING_BOARD_REV1);
	rev2 = intf->cb->gpio->map(intf, SPRING_BOARD_REV2);
	if (!rev0 || !rev1 || !rev2) {
		lprintf(LOG_DEBUG, "%s: Unable to determine board "
				"revision\n", __func__);
		return SPRING_CONFIG_UNKNOWN;
	}

	v0 = exynos5_read_gpio_mvl(intf, rev0);
	v1 = exynos5_read_gpio_mvl(intf, rev1);
	v2 = exynos5_read_gpio_mvl(intf, rev2);
	lprintf(LOG_DEBUG, "%s: v0: %u, v1: %u, v2: %u\n",
			__func__, v0, v1, v2);

	for (i = 0; i < ARRAY_SIZE(spring_id_map); i++)
		if ((v0 == spring_id_map[i].v0) &&
		    (v1 == spring_id_map[i].v1) &&
		    (v2 == spring_id_map[i].v2))
			config = spring_id_map[i].config;

	return config;
}

static int spring_setup_post(struct platform_intf *intf)
{
	spring_board_config = spring_get_board_config(intf);
	if (spring_board_config == SPRING_CONFIG_UNKNOWN)
		return -1;

	if (spring_ec_setup(intf) <= 0)
		return -1;

	return 0;
}

static int spring_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	return 0;
}

struct platform_cb spring_cb = {
	.ec 		= &gec_cb,
	.eeprom 	= &spring_eeprom_cb,
	.gpio		= &spring_gpio_cb,
	.memory		= &spring_memory_cb,
	.nvram		= &gec_nvram_cb,
	.sys 		= &spring_sys_cb,
};

struct platform_intf platform_spring = {
	.type		= PLATFORM_ARMV7,
	.name		= "Spring",
	.id_list	= spring_id_list,
	.sub		= spring_sub,
	.cb		= &spring_cb,
	.probe		= &spring_probe,
	.setup_post	= &spring_setup_post,
	.destroy	= &spring_destroy,
};
