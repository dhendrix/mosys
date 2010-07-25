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

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/eeprom_enet.h"
#include "lib/fmap.h"
#include "lib/smbios.h"

#include "drivers/intel/nm10.h"

/* set up EEPROM's MMIO location */
static int agz_pinetrail_host_firmware_setup(struct platform_intf *intf,
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

static int agz_host_firmware_read(struct platform_intf *intf,
                                  struct eeprom *eeprom,
                                  unsigned int offset,
				  unsigned int len,
                                  void *data)
{
	uint8_t *buf = data;
	enum ich_bbs bbs_orig;

	bbs_orig = nm10_get_bbs(intf);

	/* set chipset to direct TOLM firmware region I/Os to SPI */
	if (nm10_set_bbs(intf, ICH_BBS_SPI) < 0)
		return -1;

	if (eeprom_mmio_read(intf, eeprom, offset, len, buf) < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read device\n", __func__);
		return -1;
	}

	/* restore original BBS value */
	nm10_set_bbs(intf, bbs_orig);

	return 0;
}

/*
 * agz_get_host_fmap - return a fmap structure for host firmware
 *
 * @intf:	platform interface
 * @eeprom:	eeprom structure
 *
 * returns a newly allocated struct fmap if successful
 * returns NULL to indicate error
 */
static struct fmap *agz_get_host_fmap(struct platform_intf *intf,
                                      struct eeprom *eeprom)
{
	uint8_t *buf;
	struct fmap *fmap_embedded, *fmap;
	int fmap_offset, fmap_size;
	int len;

	if ((len= eeprom->device->size(intf, eeprom)) < 0)
		return NULL;

	buf = mosys_malloc(len);

	if (eeprom->device->read(intf, eeprom, 0, (unsigned int)len, buf) < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read device\n", __func__);
		return NULL;
	}

	if ((fmap_offset = fmap_find(buf, len)) < 0) {
		lprintf(LOG_DEBUG, "%s: cannot find fmap\n", __func__);
		return NULL;
	}

	fmap_embedded = (struct fmap *)(buf + fmap_offset);
	fmap_size = sizeof(*fmap) + (fmap_embedded->nareas *
	                             sizeof(fmap_embedded->areas[0]));
	fmap = malloc(fmap_size);
	memcpy(fmap, fmap_embedded, fmap_size);

	free(buf);
	return fmap;
}

static struct eeprom_dev agz_host_firmware = {
	.size		= agz_host_firmware_size,
	.read		= agz_host_firmware_read,
	.get_map	= agz_get_host_fmap,
};

static struct eeprom agz_pinetrail_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		/* FIXME: add proper address stuff here */
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &agz_host_firmware,
		.setup		= agz_pinetrail_host_firmware_setup,
	},
	{
		.name		= "ec_firmware",
		.type		= EEPROM_TYPE_FW,
		/* FIXME: add proper address stuff here */
		.flags		= EEPROM_FLAG_RDWR,
	},
	{ 0 },
};

int agz_pinetrail_eeprom_setup(struct platform_intf *intf)
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

struct eeprom_cb agz_pinetrail_eeprom_cb = {
	.eeprom_list	= agz_pinetrail_eeproms,
};
