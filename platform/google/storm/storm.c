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

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "storm.h"

enum storm_boards {
	STORM,
	WHIRLWIND,
	ARKHAM,
};

static const char *id_list[] = {
	[STORM] = "google,storm",
	[WHIRLWIND] = "google,whirlwind",
	[ARKHAM] = "google,arkham",
	NULL,
};

struct platform_cmd *storm_sub[] = {
	&cmd_eeprom,
//	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

int storm_probe(struct platform_intf *intf)
{
	int index;

	index = probe_fdt_compatible(&id_list[0], ARRAY_SIZE(id_list), 1);
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", id_list[index]);

		if (index == WHIRLWIND)
			intf->name = "Whirlwind";

		if (index == ARKHAM)
			intf->name = "Arkham";
	}


	return index >= 0 ? 1 : 0;
}

static int storm_destroy(struct platform_intf *intf)
{
	return 0;
}

struct eventlog_cb storm_eventlog_cb = {
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

struct platform_cb storm_cb = {
	.eeprom 	= &storm_eeprom_cb,
//	.gpio		= &storm_gpio_cb,
	.memory		= &storm_memory_cb,
	.nvram		= &cros_spi_flash_nvram_cb,
	.sys 		= &storm_sys_cb,
	.eventlog	= &storm_eventlog_cb,
};

struct platform_intf platform_storm = {
	.type		= PLATFORM_ARMV7,
	.name		= "Storm",
	.sub		= storm_sub,
	.cb		= &storm_cb,
	.probe		= &storm_probe,
	.destroy	= &storm_destroy,
};
