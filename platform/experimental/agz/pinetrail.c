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

#include "lib/file.h"
#include "lib/smbios.h"
#include "lib/vpd.h"

#include "pinetrail.h"

static const char *probed_platform_id;

const char *agz_pinetrail_id_list[] = {
	"AGZ",
	NULL
};

struct platform_cmd *agz_pinetrail_sub[] = {
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static char agz_hwid[] = "{9707217C-7943-4376-A812-FA05C318A16F}";
const char *agz_pinetrail_probe(struct platform_intf *intf)
{
	int fd;
	char *buf;

	if (probed_platform_id)
		return probed_platform_id;

	/*
	 * AGZ is using an experimental identifier called the "HWID". It's a
	 * GUID that is found somewhere in ACPI space and exposed via sysfs.
	 */
	fd = file_open("/sys/devices/platform/chromeos_acpi/HWID", FILE_READ);
	buf = malloc(strlen(agz_hwid));
	if (read(fd, buf, strlen(agz_hwid)) < 0) {
		lprintf(LOG_DEBUG, "%s: Could not open ACPI HWID\n", __func__);
		free(buf);
		return NULL;
	}
	lprintf(LOG_DEBUG, "%s: HWID: %s\n", __func__, buf);
	if (!strcmp(buf, agz_hwid)) {
		/* FIXME: this basically just assigns a human-readable name
		   to the platform */
		lprintf(LOG_DEBUG, "%s: Matched ACPI HWID\n", __func__);
		probed_platform_id = strdup(agz_pinetrail_id_list[0]);
	}
	free(buf);

#if 0
	/*
	 * The pinetrail reference platform uses the model presented in the
	 * SMBIOS type 1 table for identification. For this string to be
	 * found, some common ops must first be set up.
	 */
	if (intf && !intf->op)
		intf->op = &platform_common_op;

	/* Usually this level of caution isn't required, but since this is very
	   early we should be cautious... */
	if (intf && intf->cb && intf->cb->sysinfo && intf->cb->sysinfo->name)
		probed_platform_id = intf->cb->sysinfo->name(intf);

	/*
	 * if the sysinfo callback didn't work, then perhaps we can try finding
	 * the string by directly invoking smbios_find_string.
	 */
	if (!probed_platform_id) {
		probed_platform_id = smbios_find_string(intf,
		                                       SMBIOS_TYPE_SYSTEM,
		                                       1,
		                                       SMBIOS_LEGACY_ENTRY_BASE,
		                                       SMBIOS_LEGACY_ENTRY_LEN);
	}
#endif

	return probed_platform_id;
}

/* late setup routine; not critical to core functionality */
static int agz_pinetrail_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
//	rc |= agz_pinetrail_vpd_setup(intf);
	if (agz_pinetrail_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= agz_pinetrail_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int agz_pinetrail_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb agz_pinetrail_cb = {
	.eeprom		= &agz_pinetrail_eeprom_cb,
	.gpio		= &agz_pinetrail_gpio_cb,
	.nvram		= &agz_pinetrail_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sysinfo 	= &agz_pinetrail_sysinfo_cb,
	.vpd		= &agz_pinetrail_vpd_cb,
};

struct platform_intf platform_agz_pinetrail = {
	.type		= PLATFORM_X86_64,
	.name		= "AGZ",
	.id_list	= agz_pinetrail_id_list,
	.sub		= agz_pinetrail_sub,
	.cb		= &agz_pinetrail_cb,
	.probe		= &agz_pinetrail_probe,
	.setup_post	= &agz_pinetrail_setup_post,
	.destroy	= &agz_pinetrail_destroy,
};
