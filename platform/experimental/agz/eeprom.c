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

#include <stdio.h>
#include <unistd.h>

#include "mosys/platform.h"
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/eeprom_enet.h"
#include "lib/smbios.h"

static size_t agz_host_firmware_size(struct platform_intf *intf,
                                     struct eeprom *eeprom)
{
	size_t rom_size;
	struct smbios_table table;

	/*
	 * Determine size of firmware using SMBIOS. We might implement a more
	 * sophisticated approach in the future, but this works well for now
	 * since it does not depend on the BIOS boot straps value.
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

	return rom_size;
}

static struct eeprom_dev agz_host_firmware = {
	.size		= agz_host_firmware_size,
};

static struct eeprom agz_pinetrail_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		/* FIXME: add proper address stuff here */
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &agz_host_firmware,
	},
	{
		.name		= "ec_firmware",
		.type		= EEPROM_TYPE_FW,
		/* FIXME: add proper address stuff here */
		.flags		= EEPROM_FLAG_RDWR,
	},
	{ 0 },
};

struct eeprom_cb agz_pinetrail_eeprom_cb = {
	.eeprom_list	= agz_pinetrail_eeproms,
};
