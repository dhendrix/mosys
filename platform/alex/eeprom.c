/*
 * Copyright 2012, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <unistd.h>

#include <fmap.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/eeprom_enet.h"
#include "lib/smbios.h"

#include "drivers/intel/nm10.h"

/* set up EEPROM's MMIO location */
static int alex_firmware_mmio_setup(struct platform_intf *intf,
                                   struct eeprom *eeprom)
{
	unsigned long int rom_size = 0, rom_base = 0;

	if (!eeprom->device || !eeprom->device->size)
		return -1;
	rom_size = eeprom->device->size(intf);
	rom_base = 0xffffffff - rom_size + 1;
	lprintf(LOG_DEBUG, "%s: rom_base: 0x%08x, rom_size: 0x%08x\n",
	__func__, rom_base, rom_size);

	eeprom->addr.mmio = rom_base;

	return 0;
}

static int alex_firmware_read(struct platform_intf *intf,
                             struct eeprom *eeprom,
                             unsigned int offset,
                             unsigned int len,
                             void *data,
                             enum ich_bbs_ich7 bbs)
{
	uint8_t *buf = data;
	enum ich_bbs_ich7 bbs_orig;

	bbs_orig = nm10_get_bbs(intf);

	/* set chipset to direct TOLM firmware region I/Os to SPI */
	if (nm10_set_bbs(intf, bbs) < 0) {
		lprintf(LOG_DEBUG, "%s: cannot set bbs\n", __func__);
		return -1;
	}

	if (eeprom_mmio_read(intf, eeprom, offset, len, buf) < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read device\n", __func__);
		return -1;
	}

	/* restore original BBS value */
	nm10_set_bbs(intf, bbs_orig);

	return 0;
}

static int alex_host_firmware_size(struct platform_intf *intf)
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

		/* fake it */
		lprintf(LOG_INFO, "Assuming 4MB firmware ROM\n");
		rom_size = 4096 * 1024;
	} else {
		rom_size = (table.data.bios.rom_size_64k_blocks + 1) * 64 * 1024;
	}

	return rom_size;
}

static int alex_host_firmware_read(struct platform_intf *intf,
                                  struct eeprom *eeprom,
                                  unsigned int offset,
                                  unsigned int len,
                                  void *data)
{
	return alex_firmware_read(intf, eeprom, offset, len, data, ICH7_BBS_SPI);
}

static struct eeprom_dev alex_host_firmware = {
	.size		= alex_host_firmware_size,
	.read		= alex_host_firmware_read,
	.get_map	= eeprom_get_fmap,
};

static int alex_ec_firmware_size(struct platform_intf *intf)
{
	/* FIXME: the actual mechanism for obtaining this info is not yet
	 * supported by mosys, so we'll cheat for now */
	return 128 * 1024;
}

static int alex_ec_firmware_read(struct platform_intf *intf,
                                struct eeprom *eeprom,
                                unsigned int offset,
                                unsigned int len,
                                void *data)
{
	return alex_firmware_read(intf, eeprom, offset, len, data, ICH7_BBS_LPC);
}

static struct eeprom_dev alex_ec_firmware = {
	.size		= alex_ec_firmware_size,
	.read		= alex_ec_firmware_read,
	.get_map	= eeprom_get_fmap,
};

static struct eeprom samsung_series5_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &alex_host_firmware,
		.setup		= alex_firmware_mmio_setup,
	},
	{
		.name		= "ec_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &alex_ec_firmware,
		.setup		= alex_firmware_mmio_setup,
	},
	{ 0 },
};

int samsung_series5_eeprom_setup(struct platform_intf *intf)
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

struct eeprom_cb samsung_series5_eeprom_cb = {
	.eeprom_list	= samsung_series5_eeproms,
};
