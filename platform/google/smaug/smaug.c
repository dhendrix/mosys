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

#include "mosys/alloc.h"
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

#include "smaug.h"

struct board_id_gpio {
	int num;
	gpio_t *gpios;
};

const char *id_list[] = {
	"google,smaug",
	NULL,
};

struct platform_cmd *sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

int probe(struct platform_intf *intf)
{
	int index;

	index = probe_fdt_compatible(&id_list[0],
					ARRAY_SIZE(id_list), 0);
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", id_list[index]);
		return 1;
	}

	return 0;
}

enum smaug_type get_smaug_type(struct platform_intf *intf)
{
	enum smaug_type ret = SMAUG_UNKNOWN;

	if (!strncmp(intf->name, "Ryu", strlen(intf->name)))
		ret = SMAUG_RYU;

	return ret;
}

static int setup_post(struct platform_intf *intf)
{
	return cros_ec_setup(intf);
}

static int destroy(struct platform_intf *intf)
{
	if (intf->cb->ec->destroy)
		intf->cb->ec->destroy(intf, intf->cb->ec);
	return 0;
}

struct eventlog_cb smaug_eventlog_cb = {
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

struct platform_cb cb = {
	.ec		= &cros_ec_cb,
	.eeprom		= &smaug_eeprom_cb,
	.nvram		= &cros_spi_flash_nvram_cb,
	.sys		= &smaug_sys_cb,
	.eventlog	= &smaug_eventlog_cb,
};

struct platform_intf platform_smaug = {
	.type		= PLATFORM_ARMV8,
	.name		= "Ryu",
	.id_list	= id_list,
	.sub		= sub,
	.cb		= &cb,
	.probe		= &probe,
	.setup_post	= &setup_post,
	.destroy	= &destroy,
};
