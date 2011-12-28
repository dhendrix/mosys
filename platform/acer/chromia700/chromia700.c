/*
 * Copyright (C) 2010 Google Inc.
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
#include "lib/smbios.h"
#include "lib/vpd.h"

#include "chromia700.h"

static const char *probed_platform_id;

const char *acer_chromia700_id_list[] = {
	"AGZ",
	"BGZ",
	"ZGB",
	NULL
};

struct platform_cmd *acer_chromia700_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_i2c,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static const char *hwids[] = {
	"{9707217C-7943-4376-A812-FA05C318A16F}",
	"{FA42644C-CF3A-4692-A9D3-1A667CB232E9}",
	NULL
};

static const char *frids[] = {
	"ZGB",
	NULL
};

int acer_chromia700_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto acer_chromia700_probe_exit;
	}

	if (probe_frid(frids)) {
		status = 1;
		goto acer_chromia700_probe_exit;
	}

	if (probe_smbios(intf, acer_chromia700_id_list)) {
		status = 1;
		goto acer_chromia700_probe_exit;
	}

acer_chromia700_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int acer_chromia700_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
//	rc |= acer_chromia700_vpd_setup(intf);
	if (acer_chromia700_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= acer_chromia700_ec_setup(intf);
	rc |= acer_chromia700_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int acer_chromia700_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	acer_chromia700_ec_destroy(intf);
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb acer_chromia700_cb = {
	.ec		= &acer_chromia700_ec_cb,
	.eeprom		= &acer_chromia700_eeprom_cb,
	.gpio		= &acer_chromia700_gpio_cb,
	.memory		= &acer_chromia700_memory_cb,
	.nvram		= &acer_chromia700_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &acer_chromia700_sys_cb,
	.vpd		= &acer_chromia700_vpd_cb,
};

struct platform_intf platform_acer_chromia700 = {
	.type		= PLATFORM_X86_64,
	.name		= "ZGB",
	.id_list	= acer_chromia700_id_list,
	.sub		= acer_chromia700_sub,
	.cb		= &acer_chromia700_cb,
	.probe		= &acer_chromia700_probe,
	.setup_post	= &acer_chromia700_setup_post,
	.destroy	= &acer_chromia700_destroy,
};
