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
#include "lib/smbios.h"

#include "drivers/intel/series6.h"
#include "drivers/intel/nm10.h"

#include "parrot.h"

/* set up EEPROM's MMIO location */
static int parrot_firmware_mmio_setup(struct platform_intf *intf,
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

static int parrot_firmware_read(struct platform_intf *intf,
                             struct eeprom *eeprom,
                             unsigned int offset,
                             unsigned int len,
                             void *data,
                             enum ich_snb_bbs bbs)
{
	uint8_t *buf = data;
	enum ich_snb_bbs bbs_orig;

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

static size_t parrot_host_firmware_size(struct platform_intf *intf,
                                     struct eeprom *eeprom)
{
	/*
	 * FIXME: use libflashrom for this. SMBIOS won't work because it
	 * reports the actual BIOS rather than the ROM size which includes
	 * ME/GbE/etc.
	 */
	return PARROT_HOST_FIRMWARE_ROM_SIZE;
}

static int parrot_host_firmware_read(struct platform_intf *intf,
                                  struct eeprom *eeprom,
                                  unsigned int offset,
                                  unsigned int len,
                                  void *data)
{
	return parrot_firmware_read(intf, eeprom, offset,
	                            len, data, ICH_SNB_BBS_SPI);
}

static struct eeprom_dev parrot_host_firmware = {
	.size		= parrot_host_firmware_size,
	.read		= parrot_host_firmware_read,
	.get_map	= eeprom_get_fmap,
};

static struct eeprom parrot_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &parrot_host_firmware,
		.setup		= parrot_firmware_mmio_setup,
	},
	{ 0 },
};

int parrot_eeprom_setup(struct platform_intf *intf)
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

struct eeprom_cb parrot_eeprom_cb = {
	.eeprom_list	= parrot_eeproms,
};
