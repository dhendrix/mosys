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
#include "drivers/nvidia/tegra124/gpio.h"

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "nyan.h"

const char *nyan_id_list[] = {
	"google,nyan",
	NULL,
};

const char *nyan_big_id_list[] = {
	"google,nyan-big",
	NULL,
};

const char *nyan_blaze_id_list[] = {
	"google,nyan-blaze-rev5",
	"google,nyan-blaze-rev4",
	"google,nyan-blaze-rev3",
	"google,nyan-blaze-rev2",
	"google,nyan-blaze-rev1",
	"google,nyan-blaze-rev0",
	NULL,
};

const char *nyan_kitty_id_list[] = {
	"google,nyan-kitty-rev5",
	"google,nyan-kitty-rev4",
	"google,nyan-kitty-rev3",
	"google,nyan-kitty-rev2",
	"google,nyan-kitty-rev1",
	"google,nyan-kitty-rev0",
	NULL,
};

struct platform_cmd *nyan_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_psu,
	&cmd_eventlog,
	NULL
};

int nyan_probe(struct platform_intf *intf)
{
	int index;

	/* nyan-big is listed before google,nyan, so search for it first */
	index = probe_fdt_compatible(&nyan_big_id_list[0],
					ARRAY_SIZE(nyan_big_id_list), 0);
	if (index >= 0) {
		gpio_t gpio[] = {GPIO(Q3), GPIO(T1), GPIO(X1), GPIO(X4)};
		int value;
		char *str =strdup("google,nyan-big-rev%d");

		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_big_id_list[index]);
		intf->name = "Big";

		gpio_get_in_tristate_values(intf, gpio, ARRAY_SIZE(gpio));

		value = gpio_get_in_tristate_values(intf, gpio, ARRAY_SIZE(gpio));
		sprintf(str, "google,nyan-big-rev%d", value);
		intf->version_id = str;

		return 1;
	}

	/* nyan-blaze is listed before google,nyan, so search for it first */
	index = probe_fdt_compatible(&nyan_blaze_id_list[0],
					ARRAY_SIZE(nyan_blaze_id_list), 0);
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_blaze_id_list[index]);
		intf->name = "Blaze";
		return 1;
	}

	/* nyan-kitty is listed before google,nyan, so search for it first */
	index = probe_fdt_compatible(&nyan_kitty_id_list[0],
					ARRAY_SIZE(nyan_kitty_id_list), 0);
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_kitty_id_list[index]);
		intf->name = "Kitty";
		return 1;
	}

	index = probe_fdt_compatible(&nyan_id_list[0],
					ARRAY_SIZE(nyan_id_list), 0);
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_id_list[index]);
		return 1;
	}

	return 0;
}

enum nyan_type get_nyan_type(struct platform_intf *intf)
{
	enum nyan_type ret = NYAN_UNKNOWN;

	if (!strncmp(intf->name, "Big", strlen(intf->name)))
		ret = NYAN_BIG;
	else if (!strncmp(intf->name, "Blaze", strlen(intf->name)))
		ret = NYAN_BLAZE;
	else if (!strncmp(intf->name, "Kitty", strlen(intf->name)))
		ret = NYAN_KITTY;
	else if (!strncmp(intf->name, "Nyan", strlen(intf->name)))
		ret = NYAN;

	return ret;
}

static int nyan_setup_post(struct platform_intf *intf)
{
	return cros_ec_setup(intf);
}

static int nyan_destroy(struct platform_intf *intf)
{
	if (intf->cb->ec->destroy)
		intf->cb->ec->destroy(intf, intf->cb->ec);
	return 0;
}

struct eventlog_cb nyan_eventlog_cb = {
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

struct platform_cb nyan_cb = {
	.ec 		= &cros_ec_cb,
	.eeprom 	= &nyan_eeprom_cb,
	.memory		= &nyan_memory_cb,
	.nvram		= &cros_ec_nvram_cb,
	.psu		= &nyan_psu_cb,
	.sys 		= &nyan_sys_cb,
	.eventlog	= &nyan_eventlog_cb,
};

struct platform_intf platform_nyan = {
	.type		= PLATFORM_ARMV7,
	.name		= "Nyan",
	.id_list	= nyan_id_list,
	.sub		= nyan_sub,
	.cb		= &nyan_cb,
	.probe		= &nyan_probe,
	.setup_post	= &nyan_setup_post,
	.destroy	= &nyan_destroy,
	.version_id   = "google,nyan",
};
