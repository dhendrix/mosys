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

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/smbios.h"
#include "lib/vpd.h"

#include "lumpy.h"

int lumpy_vpd_setup(struct platform_intf *intf)
{
	unsigned int rom_base, rom_size;

	/* FIXME: SMBIOS might not be useful for ROM size detection here since
	   BIOS size will only reflect the size of the BIOS flash partition */
	rom_size = LUMPY_HOST_FIRMWARE_ROM_SIZE;
	rom_base = 0xffffffff - rom_size + 1;
	vpd_rom_base = rom_base;
	vpd_rom_size = rom_size;

	return 0;
}

static char *lumpy_vpd_get_serial(struct platform_intf *intf)
{
	if (intf->cb->vpd && intf->cb->vpd->system_serial)
		return intf->cb->vpd->system_serial(intf);
	else
		return NULL;

}

static char *lumpy_vpd_get_sku(struct platform_intf *intf)
{
	if (intf->cb->vpd && intf->cb->vpd->system_sku)
		return intf->cb->vpd->system_sku(intf);
	else
		return NULL;
}


static char *lumpy_vpd_get_google_hwqualid(struct platform_intf *intf)
{
	/* FIXME: noop for now */
	return NULL;
}

struct vpd_cb lumpy_vpd_cb = {
	.system_serial		= &lumpy_vpd_get_serial,
	.system_sku		= &lumpy_vpd_get_sku,
	.google_hwqualid	= &lumpy_vpd_get_google_hwqualid,
};
