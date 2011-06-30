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

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/probe.h"
//#include "lib/vpd.h"

#include "aebl.h"

static const char *probed_platform_id;

const char *aebl_tegra2_id_list[] = {
	"Aebl",
	NULL
};

struct platform_cmd *aebl_tegra2_sub[] = {
//	&cmd_eeprom,
//	&cmd_gpio,
//	&cmd_i2c,
//	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
//	&cmd_vpd,
	NULL
};

#if 0
static const char *hwids[] = {
	NULL
};
#endif

int aebl_tegra2_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

#if 0
	if (probe_hwid(hwids)) {
		status = 1;
		goto aebl_tegra2_probe_exit;
	}
#endif

	if (probe_cpuinfo(intf, "Hardware", "aebl")) {
		status = 1;
		goto aebl_tegra2_probe_exit;
	}

aebl_tegra2_probe_exit:
	probed = 1;
	return status;
}

#if 0
/* late setup routine; not critical to core functionality */
static int aebl_tegra2_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
//	rc |= aebl_tegra2_vpd_setup(intf);
	if (aebl_tegra2_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= aebl_tegra2_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}
#endif

static int aebl_tegra2_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb aebl_tegra2_cb = {
//	.eeprom		= &aebl_tegra2_eeprom_cb,
//	.gpio		= &aebl_tegra2_gpio_cb,
//	.memory		= &aebl_tegra2_memory_cb,
//	.nvram		= &aebl_tegra2_nvram_cb,
	.sysinfo 	= &aebl_tegra2_sysinfo_cb,
//	.vpd		= &aebl_tegra2_vpd_cb,
};

struct platform_intf platform_aebl_tegra2 = {
	.type		= PLATFORM_ARMV7,
	.name		= "Aebl",
	.id_list	= aebl_tegra2_id_list,
	.sub		= aebl_tegra2_sub,
	.cb		= &aebl_tegra2_cb,
	.probe		= &aebl_tegra2_probe,
//	.setup_post	= &aebl_tegra2_setup_post,
	.destroy	= &aebl_tegra2_destroy,
};
