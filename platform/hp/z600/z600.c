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

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/smbios.h"

#include "z600.h"

static const char *probed_platform_id;

const char *hp_z600_id_list[] = {
	"HP Z600 Workstation",
	NULL
};

struct platform_cmd *platform_hp_z600_sub[] = {
	&cmd_eeprom,
	&cmd_platform,
	&cmd_smbios,
	NULL
};

const char *hp_z600_probe(struct platform_intf *intf)
{
	if (probed_platform_id)
		return probed_platform_id;

	/*
	 * The z600 uses the model presented in the SMBIOS type 1 table
	 * for identification. For this string to be found, some common ops
	 * must first be set up.
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

	return probed_platform_id;
}

static int hp_z600_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	return 0;
}

struct platform_cb hp_z600_cb = {
	.eeprom		= &hp_z600_eeprom_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sysinfo 	= &hp_z600_sysinfo_cb,
};

struct platform_intf platform_hp_z600 = {
	.type		= PLATFORM_X86_64,
	.name		= "HP Z600 Workstation",
	.id_list	= hp_z600_id_list,
	.sub		= platform_hp_z600_sub,
	.cb		= &hp_z600_cb,
	.probe		= hp_z600_probe,
	.destroy	= &hp_z600_destroy,
};
