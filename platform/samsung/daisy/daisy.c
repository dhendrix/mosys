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

#include <stdlib.h>
#include <unistd.h>

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/google/gec.h"
#include "drivers/samsung/exynos5250/gpio.h"

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "daisy.h"

static const char *probed_platform_id;
enum daisy_board_config board_config;

const char *daisy_id_list[] = {
	"Daisy",
	"Google Daisy",
	"Google Snow",
	"SMDK5250",
	"Snow",
	NULL
};

struct platform_cmd *daisy_sub[] = {
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

int daisy_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;
	char *model = NULL;

	if (probed)
		return status;

	model = model_from_fdt();

	for (id = daisy_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto daisy_probe_exit;
		}

		if (model) {
			lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ", model, *id);
			if (!strcmp(*id, model)) {
				lprintf(LOG_DEBUG, "yes\n");
				status = 1;
				goto daisy_probe_exit;
			} else {
				lprintf(LOG_DEBUG, "no\n");
			}
		}
	}

#if 0
	if (probe_hwid(hwids)) {
		status = 1;
		goto daisy_probe_exit;
	}
#endif

daisy_probe_exit:
	probed = 1;
	return status;
}

struct {
	enum mvl3 v0, v1;
	enum daisy_board_config config;
} snow_id_map[] = {
	/*  ID0      ID1         config */
	{ LOGIC_0, LOGIC_0, SNOW_CONFIG_SAMSUNG_MP_1_2 },
	{ LOGIC_0, LOGIC_1, SNOW_CONFIG_ELPIDA_MP },
	{ LOGIC_1, LOGIC_0, SNOW_CONFIG_SAMSUNG_DVT },
	{ LOGIC_1, LOGIC_1, SNOW_CONFIG_ELPIDA_DVT },
	{ LOGIC_0, LOGIC_Z, SNOW_CONFIG_SAMSUNG_PVT },
	{ LOGIC_1, LOGIC_Z, SNOW_CONFIG_ELPIDA_PVT },
	{ LOGIC_Z, LOGIC_0, SNOW_CONFIG_SAMSUNG_MP },
	{ LOGIC_Z, LOGIC_Z, SNOW_CONFIG_ELPIDA_MP },
	{ LOGIC_Z, LOGIC_1, SNOW_CONFIG_RSVD },
};

static int snow_get_board_config(struct platform_intf *intf)
{
	int i;
	struct gpio_map *id0, *id1;
	enum mvl3 v0, v1;
	enum daisy_board_config config = SNOW_CONFIG_UNKNOWN;

	id0 = intf->cb->gpio->map(intf, SNOW_BOARD_ID0);
	id1 = intf->cb->gpio->map(intf, SNOW_BOARD_ID1);
	if (!id0 || !id1) {
		lprintf(LOG_DEBUG, "%s: Unable to determine id0/1\n", __func__);
		return SNOW_CONFIG_UNKNOWN;
	}

	v0 = exynos5250_read_gpio_mvl(intf, id0);
	v1 = exynos5250_read_gpio_mvl(intf, id1);
	lprintf(LOG_DEBUG, "%s: v0: %u, v1: %u\n", __func__, v0, v1);

	for (i = 0; i < ARRAY_SIZE(snow_id_map); i++) {
		if (v0 == snow_id_map[i].v0 && v1 == snow_id_map[i].v1)
			config = snow_id_map[i].config;
	}

	return config;
}

static int daisy_setup_post(struct platform_intf *intf)
{
	const char *name;

	if (daisy_ec_setup(intf) <= 0)
		return -1;

	/*
	 * FIXME: This is a hack that overrides the "Daisy" canonical platform
	 * name with "Snow" depending on the family of EC which is detected.
	 * Daisy is expected to use stm32l, and Snow is expected to use stm32f.
	 *
	 * If name == NULL, it means communication with the EC failed somehow.
	 */
	name = intf->cb->ec->name(intf);
	if (!name) {
		lprintf(LOG_ERR, "Unable to determine platform variant\n");
		return -1;
	}
	if (!strncmp(name, "stm32f", 6)) {
		lprintf(LOG_DEBUG, "Overriding platform name %s with %s\n",
			intf->name, "Snow");
		intf->name = "Snow";

		board_config = snow_get_board_config(intf);
	}

	return 0;
}

static int daisy_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	return 0;
}

struct platform_cb daisy_cb = {
	.ec 		= &gec_cb,
	.eeprom 	= &daisy_eeprom_cb,
	.gpio		= &daisy_gpio_cb,
	.memory		= &daisy_memory_cb,
	.nvram		= &gec_nvram_cb,
	.sys 		= &daisy_sys_cb,
};

struct platform_intf platform_daisy = {
	.type		= PLATFORM_ARMV7,
	.name		= "Daisy",
	.id_list	= daisy_id_list,
	.sub		= daisy_sub,
	.cb		= &daisy_cb,
	.probe		= &daisy_probe,
	.setup_post	= &daisy_setup_post,
	.destroy	= &daisy_destroy,
};
