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

#include "stumpy.h"

static const char *probed_platform_id;

const char *stumpy_id_list[] = {
	"Stumpy",
	NULL
};

struct platform_cmd *stumpy_sub[] = {
	&cmd_eeprom,
//	&cmd_gpio,
//	&cmd_i2c,
	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
	&cmd_sensor,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static const char *hwids[] = {
	"X86 STUMPY",
};

int stumpy_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto stumpy_probe_exit;
	}

	if (probe_smbios(intf, stumpy_id_list)) {
		status = 1;
		goto stumpy_probe_exit;
	}

stumpy_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int stumpy_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (stumpy_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= stumpy_eeprom_setup(intf);
	rc |= stumpy_superio_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int stumpy_destroy(struct platform_intf *intf)
{
	stumpy_superio_destroy(intf);

	if (probed_platform_id)
		free((char *)probed_platform_id);

	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb stumpy_cb = {
	.eeprom		= &stumpy_eeprom_cb,
//	.gpio		= &stumpy_gpio_cb,
	.memory		= &stumpy_memory_cb,
//	.nvram		= &stumpy_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sensor		= &stumpy_sensor_cb,
	.sysinfo 	= &stumpy_sysinfo_cb,
	.vpd		= &stumpy_vpd_cb,
};

struct platform_intf platform_stumpy = {
	.type		= PLATFORM_X86_64,
	.name		= "Stumpy",
	.id_list	= stumpy_id_list,
	.sub		= stumpy_sub,
	.cb		= &stumpy_cb,
	.probe		= &stumpy_probe,
	.setup_post	= &stumpy_setup_post,
	.destroy	= &stumpy_destroy,
};
