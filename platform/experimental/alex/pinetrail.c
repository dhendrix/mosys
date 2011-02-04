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

#include "mosys/alloc.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/probe.h"
#include "lib/smbios.h"
#include "lib/vpd.h"

#include "pinetrail.h"

static const char *probed_platform_id;

const char *alex_pinetrail_id_list[] = {
	"Alex",
	NULL
};

struct platform_cmd *alex_pinetrail_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_i2c,
//	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static const char *hwids[] = {
	"{97A1FBD6-FDE1-4FC5-BB81-286608B90FCE}",
	NULL
};

int alex_pinetrail_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto alex_pinetrail_probe_exit;
	}

	if (probe_smbios(intf, alex_pinetrail_id_list)) {
		status = 1;
		goto alex_pinetrail_probe_exit;
	}

alex_pinetrail_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int alex_pinetrail_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (alex_pinetrail_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	if (alex_pinetrail_ec_setup(intf) < 0) {
		lprintf(LOG_WARNING, "Non-fatal error: Failed to setup "
		                     "EC callbacks.\n");

	}
	rc |= alex_pinetrail_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int alex_pinetrail_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	alex_pinetrail_ec_destroy(intf);
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb alex_pinetrail_cb = {
	.ec		= &alex_pinetrail_ec_cb,
	.eeprom		= &alex_pinetrail_eeprom_cb,
	.gpio		= &alex_pinetrail_gpio_cb,
//	.nvram		= &alex_pinetrail_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sysinfo 	= &alex_pinetrail_sysinfo_cb,
	.vpd		= &alex_pinetrail_vpd_cb,
};

struct platform_intf platform_alex_pinetrail = {
	.type		= PLATFORM_X86_64,
	.name		= "alex",
	.id_list	= alex_pinetrail_id_list,
	.sub		= alex_pinetrail_sub,
	.cb		= &alex_pinetrail_cb,
	.probe		= &alex_pinetrail_probe,
	.setup_post	= &alex_pinetrail_setup_post,
	.destroy	= &alex_pinetrail_destroy,
};
