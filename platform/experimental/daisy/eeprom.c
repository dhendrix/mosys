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

#include "drivers/google/gec.h"
#include "drivers/google/gec_ec_commands.h"

#include "lib/eeprom.h"
#include "lib/flashrom.h"

#include "daisy.h"

#define DAISY_HOST_FIRMWARE_ROM_SIZE	(4096 * 1024)

static size_t daisy_host_firmware_size(struct platform_intf *intf,
				       struct eeprom *eeprom)
{
	return DAISY_HOST_FIRMWARE_ROM_SIZE;
}

static int daisy_host_firmware_read(struct platform_intf *intf,
				    struct eeprom *eeprom,
				    unsigned int offset,
				    unsigned int len,
				    void *data)
{
	uint8_t *buf;
	size_t rom_size;

	rom_size = eeprom->device->size(intf, eeprom);
	buf = mosys_malloc(rom_size);

	if (flashrom_read(buf, rom_size, INTERNAL_BUS_SPI, NULL) < 0)
		return -1;

	lprintf(LOG_DEBUG, "done\n");
	memcpy(data, &buf[offset], len);
	free(buf);
	return 0;
}

static struct eeprom_dev daisy_host_firmware = {
	.size		= daisy_host_firmware_size,
	.read		= daisy_host_firmware_read,
	.get_map	= eeprom_get_fmap,
};

static size_t daisy_ec_firmware_size(struct platform_intf *intf,
				     struct eeprom *eeprom)
{
	struct ec_response_flash_info info;

	if (gec_flash_info(intf, &info) < 0) {
		lprintf(LOG_ERR, "%s: Failed to obtain flash info\n", __func__);
		return 0;
	}

	return info.flash_size;
}

static int daisy_ec_firmware_read(struct platform_intf *intf,
				  struct eeprom *eeprom,
				  unsigned int offset,
				  unsigned int len,
				  void *data)
{
	uint8_t *buf;
	size_t rom_size;

	rom_size = eeprom->device->size(intf, eeprom);
	buf = mosys_malloc(rom_size);

	if (flashrom_read(buf, rom_size, INTERNAL_BUS_I2C, NULL) < 0)
		return -1;

	lprintf(LOG_DEBUG, "done\n");
	memcpy(data, &buf[offset], len);
	free(buf);
	return 0;
}

static struct eeprom_dev daisy_ec_firmware = {
	.size		= daisy_ec_firmware_size,
	.read		= daisy_ec_firmware_read,
	.get_map	= eeprom_get_fmap,
};

static struct eeprom daisy_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &daisy_host_firmware,
	},
	{
		.name		= "ec_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR,
		.device		= &daisy_ec_firmware,
	},
	{ 0 },
};

int daisy_eeprom_setup(struct platform_intf *intf)
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

struct eeprom_cb daisy_eeprom_cb = {
	.eeprom_list	= daisy_eeproms,
};
