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

#include "lib/probe.h"
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

int hp_z600_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_smbios(intf, hp_z600_id_list)) {
		status = 1;
		goto z600_pinetrail_probe_exit;
	}

z600_pinetrail_probe_exit:
	probed = 1;
	return status;
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
