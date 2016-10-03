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

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "drivers/google/cros_ec.h"

#include "lib/fdt.h"
#include "lib/file.h"
#include "lib/generic_callbacks.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "oak.h"

static int probed_board = -1;

struct oak_probe_id {
	const char *name;
	const char *fdt_compat;
	int has_pd;
} oak_id_list[] = {
	{ "Elm", "google,elm", 0 },
	{ "Hana", "google,hana", 0 },
	{ "Oak", "google,oak", 1 },
};

#define OAK_CMD_PD_NUM	0

struct platform_cmd *oak_sub[] = {
	/* Keep this as the first entry. intf->sub will be set to point to
         * the next entry if it turns out that we don't have a PD. */
	[OAK_CMD_PD_NUM] = &cmd_pd,

	&cmd_ec,
	&cmd_eeprom,
//	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_psu,
	&cmd_eventlog,
	NULL
};

static int oak_probe(struct platform_intf *intf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(oak_id_list); i++) {
		const char **compat = &oak_id_list[i].fdt_compat;

		if (probe_fdt_compatible(compat, 1, 1) == 0) {
			lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT "
				"compatible node.\n", oak_id_list[i].name);
			intf->name = oak_id_list[i].name;
			probed_board = i;
			break;
		}
	}

	lprintf(LOG_DEBUG, "%s: probed_board: %d\n", __func__, probed_board);
	return probed_board > -1 ? 1 : 0;
}

static int oak_setup_post(struct platform_intf *intf)
{
	if (oak_ec_setup(intf) <= 0)
		return -1;

	if (oak_id_list[probed_board].has_pd) {
		if (oak_pd_setup(intf) <= 0)
			return -1;
	} else {
		intf->cb->pd = NULL;
		intf->sub = &oak_sub[OAK_CMD_PD_NUM + 1];
	}

	if (fdt_set_nvram_cb(intf) < 0)
		return -1;

	return 0;
}

static int oak_destroy(struct platform_intf *intf)
{
	if (intf->cb->ec && intf->cb->ec->destroy)
		intf->cb->ec->destroy(intf, intf->cb->ec);
	return 0;
}

struct eventlog_cb oak_eventlog_cb = {
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

struct platform_cb oak_cb = {
	.ec		= &cros_ec_cb,
	.eeprom 	= &oak_eeprom_cb,
//	.gpio		= &oak_gpio_cb,
	.memory		= &oak_memory_cb,
	.pd		= &cros_pd_cb,
	.psu		= &generic_psu_battery_cb,
	.sys		= &oak_sys_cb,
	.eventlog	= &oak_eventlog_cb,
};

struct platform_intf platform_oak = {
	.type		= PLATFORM_ARMV8,
	.name		= "Oak",
	.id_list	= oak_id_list,
	.sub		= oak_sub,
	.cb		= &oak_cb,
	.probe		= &oak_probe,
	.setup_post	= &oak_setup_post,
	.destroy	= &oak_destroy,
	.version_id	= "google,oak",
};
