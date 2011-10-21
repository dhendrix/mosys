/*
 * Copyright (C) 2011 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdlib.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/probe.h"
#include "lib/smbios.h"
#include "lib/vpd.h"

#include "lumpy.h"

static const char *probed_platform_id;

const char *lumpy_id_list[] = {
	"Lumpy",
	NULL
};

struct platform_cmd *lumpy_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
//	&cmd_gpio,
//	&cmd_i2c,
	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static const char *hwids[] = {
	"X86 LUMPY",
};

int lumpy_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto lumpy_probe_exit;
	}

	if (probe_smbios(intf, lumpy_id_list)) {
		status = 1;
		goto lumpy_probe_exit;
	}

lumpy_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int lumpy_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (lumpy_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= lumpy_eeprom_setup(intf);
	rc |= lumpy_ec_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int lumpy_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	lumpy_ec_destroy(intf);
	return 0;
}

struct platform_cb lumpy_cb = {
	.ec		= &lumpy_ec_cb,
	.eeprom		= &lumpy_eeprom_cb,
//	.gpio		= &lumpy_gpio_cb,
	.memory		= &lumpy_memory_cb,
//	.nvram		= &lumpy_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sysinfo 	= &lumpy_sysinfo_cb,
	.vpd		= &lumpy_vpd_cb,
};

struct platform_intf platform_lumpy = {
	.type		= PLATFORM_X86_64,
	.name		= "Lumpy",
	.id_list	= lumpy_id_list,
	.sub		= lumpy_sub,
	.cb		= &lumpy_cb,
	.probe		= &lumpy_probe,
	.setup_post	= &lumpy_setup_post,
	.destroy	= &lumpy_destroy,
};
