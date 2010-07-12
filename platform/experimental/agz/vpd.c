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

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/smbios.h"
#include "lib/vpd.h"

int agz_pinetrail_vpd_setup(struct platform_intf *intf)
{
	struct smbios_table table;
	unsigned int rom_base, rom_size;

	/*
	 * In this example, we'll pretend like there is VPD stored in the
	 * system firmware ROM. The ROM address is given by SMBIOS tables,
	 * though later we may wish to use something more sophisticated that
	 * probes the firmware ROM directly, like Flashrom does.
	 */
	if (smbios_find_table(intf, SMBIOS_TYPE_BIOS, 0, &table,
	                      SMBIOS_LEGACY_ENTRY_BASE,
	                      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_INFO, "Unable to calculate VPD address\n");

		/*
		 * FIXME: SMBIOS on this platform is a bit wonky at the moment,
		 * so we'll just fake it knowing the firmware ROM is 4MB.
		 */
		lprintf(LOG_INFO, "Assuming 4MB firmware ROM\n");
		rom_size = 4096 * 1024;
	} else {
		rom_size = (table.data.bios.rom_size_64k_blocks + 1) * 64 * 1024;
	}

	rom_base = 0xffffffff - rom_size + 1;
	vpd_rom_base = rom_base;
	vpd_rom_size = rom_size;

	return 0;
}

static char *agz_pinetrail_vpd_get_serial(struct platform_intf *intf)
{
	/*
	 * FIXME: This is a mock-up since this platform does not have gvpd.
	 * Serial number is available from SMBIOS table for this platform
	 */ 
	if (intf->cb->smbios && intf->cb->smbios->system_serial)
		return intf->cb->smbios->system_serial(intf);
	else
		return NULL;

}

static char *agz_pinetrail_vpd_get_sku(struct platform_intf *intf)
{
	/*
	 * FIXME: This is a mock-up since this platform does not have gvpd.
	 * SKU number is available from SMBIOS table for this platform
	 */ 
	if (intf->cb->smbios && intf->cb->smbios->system_sku)
		return intf->cb->smbios->system_sku(intf);
	else
		return NULL;
}


static char *agz_pinetrail_vpd_get_google_hwqualid(struct platform_intf *intf)
{
	/* FIXME: noop for now */
	return NULL;
}

struct vpd_cb agz_pinetrail_vpd_cb = {
	.system_serial		= &agz_pinetrail_vpd_get_serial,
	.system_sku		= &agz_pinetrail_vpd_get_sku,
	.google_hwqualid	= &agz_pinetrail_vpd_get_google_hwqualid,
};
