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

const char *samsung_series5_id_list[] = {
	"Alex",
	NULL
};

struct platform_cmd *samsung_series5_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_i2c,
	&cmd_memory,
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

int samsung_series5_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto samsung_series5_probe_exit;
	}

	if (probe_smbios(intf, samsung_series5_id_list)) {
		status = 1;
		goto samsung_series5_probe_exit;
	}

samsung_series5_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int samsung_series5_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (samsung_series5_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	if (samsung_series5_ec_setup(intf) < 0) {
		lprintf(LOG_WARNING, "Non-fatal error: Failed to setup "
		                     "EC callbacks.\n");

	}
	rc |= samsung_series5_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int samsung_series5_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	samsung_series5_ec_destroy(intf);
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb samsung_series5_cb = {
	.ec		= &samsung_series5_ec_cb,
	.eeprom		= &samsung_series5_eeprom_cb,
	.gpio		= &samsung_series5_gpio_cb,
	.memory		= &samsung_series5_memory_cb,
//	.nvram		= &samsung_series5_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sysinfo 	= &samsung_series5_sysinfo_cb,
	.vpd		= &samsung_series5_vpd_cb,
};

struct platform_intf platform_samsung_series5 = {
	.type		= PLATFORM_X86_64,
	.name		= "alex",
	.id_list	= samsung_series5_id_list,
	.sub		= samsung_series5_sub,
	.cb		= &samsung_series5_cb,
	.probe		= &samsung_series5_probe,
	.setup_post	= &samsung_series5_setup_post,
	.destroy	= &samsung_series5_destroy,
};
