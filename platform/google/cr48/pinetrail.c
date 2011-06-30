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

const char *google_cr48_id_list[] = {
	"Mario",
	NULL
};

struct platform_cmd *google_cr48_sub[] = {
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_i2c,
	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	&cmd_ec,
	NULL
};

static const char *hwids[] = {
	"{D3178EA2-58C9-4DD7-9676-95DBF45290BB}",
	"{9D799111-A88A-439E-9E1F-FBBB41B00A9A}",
	NULL
};

int google_cr48_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto google_cr48_probe_exit;
	}

	if (probe_smbios(intf, google_cr48_id_list)) {
		status = 1;
		goto google_cr48_probe_exit;
	}

google_cr48_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int google_cr48_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (google_cr48_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= google_cr48_eeprom_setup(intf);
	rc |= google_cr48_ec_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int google_cr48_destroy(struct platform_intf *intf)
{
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb google_cr48_cb = {
	.ec		= &google_cr48_ec_cb,
	.eeprom		= &google_cr48_eeprom_cb,
	.gpio		= &google_cr48_gpio_cb,
	.memory		= &google_cr48_memory_cb,
//	.nvram		= &google_cr48_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sysinfo 	= &google_cr48_sysinfo_cb,
	.vpd		= &google_cr48_vpd_cb,
};

struct platform_intf platform_google_cr48 = {
	.type		= PLATFORM_X86_64,
	.name		= "Mario",
	.id_list	= google_cr48_id_list,
	.sub		= google_cr48_sub,
	.cb		= &google_cr48_cb,
	.probe		= &google_cr48_probe,
	.setup_post	= &google_cr48_setup_post,
	.destroy	= &google_cr48_destroy,
};
