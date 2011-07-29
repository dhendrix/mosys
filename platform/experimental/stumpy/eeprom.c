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

#include <stdio.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/fmap.h"
#include "lib/smbios.h"

#include "drivers/intel/series6.h"
#include "drivers/intel/nm10.h"

#include "stumpy.h"

/* set up EEPROM's MMIO location */
static int stumpy_firmware_mmio_setup(struct platform_intf *intf,
                                   struct eeprom *eeprom)
{
	unsigned long int rom_size = 0, rom_base = 0;

	if (!eeprom->device || !eeprom->device->size)
		return -1;
	rom_size = eeprom->device->size(intf, eeprom);
	rom_base = 0xffffffff - rom_size + 1;
	lprintf(LOG_DEBUG, "%s: rom_base: 0x%08x, rom_size: 0x%08x\n",
	__func__, rom_base, rom_size);

	eeprom->addr.mmio = rom_base;

	return 0;
}

static int stumpy_firmware_read(struct platform_intf *intf,
                             struct eeprom *eeprom,
                             unsigned int offset,
                             unsigned int len,
                             void *data,
                             enum ich_bbs bbs)
{
	uint8_t *buf = data;
	enum ich_bbs bbs_orig;

	bbs_orig = series6_get_bbs(intf);

	/* set chipset to direct TOLM firmware region I/Os to SPI */
	if (series6_set_bbs(intf, bbs) < 0) {
		lprintf(LOG_DEBUG, "%s: cannot set bbs\n", __func__);
		return -1;
	}

	if (eeprom_mmio_read(intf, eeprom, offset, len, buf) < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read device\n", __func__);
		return -1;
	}

	/* restore original BBS value */
	series6_set_bbs(intf, bbs_orig);

	return 0;
}

static size_t stumpy_host_firmware_size(struct platform_intf *intf,
                                     struct eeprom *eeprom)
{
	/*
	 * FIXME: use libflashrom for this. SMBIOS won't work because it
	 * reports the actual BIOS rather than the ROM size which includes
	 * ME/GbE/etc.
	 */
	return STUMPY_HOST_FIRMWARE_ROM_SIZE;
}

static int stumpy_host_firmware_read(struct platform_intf *intf,
                                  struct eeprom *eeprom,
                                  unsigned int offset,
                                  unsigned int len,
                                  void *data)
{
	return stumpy_firmware_read(intf, eeprom, offset, len, data, ICH_BBS_SPI);
}

static struct eeprom_dev stumpy_host_firmware = {
	.size		= stumpy_host_firmware_size,
	.read		= stumpy_host_firmware_read,
	.get_map	= eeprom_get_fmap,
};

static struct eeprom stumpy_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &stumpy_host_firmware,
		.setup		= stumpy_firmware_mmio_setup,
	},
	{ 0 },
};

int stumpy_eeprom_setup(struct platform_intf *intf)
{
	struct eeprom *eeprom;
	int rc = 0;

	for (eeprom = intf->cb->eeprom->eeprom_list;
	     eeprom && eeprom->name;
	     eeprom++) {
		if (eeprom->setup)
			rc |= eeprom->setup(intf, eeprom);
	}

	return rc;
}

struct eeprom_cb stumpy_eeprom_cb = {
	.eeprom_list	= stumpy_eeproms,
};
