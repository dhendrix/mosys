/*
 * Copyright 2016, Google Inc.
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
#include "lib/flashrom.h"
#include "lib/smbios.h"

#include "reef.h"


static int host_firmware_size(struct platform_intf *intf)
{
	return REEF_HOST_FIRMWARE_ROM_SIZE;
}

static int host_firmware_read(struct platform_intf *intf,
			      struct eeprom *eeprom,
			      unsigned int offset,
			      unsigned int len,
			      void *data)
{
	uint8_t *buf;
	size_t rom_size;

	rom_size = eeprom->device->size(intf);
	buf = mosys_malloc(rom_size);

	if (flashrom_read(buf, rom_size, HOST_FIRMWARE, NULL) < 0)
		return -1;

	memcpy(data, &buf[offset], len);
	free(buf);
	return 0;
}

static int host_firmware_read_by_name(struct platform_intf *intf,
				      struct eeprom *eeprom,
				      const char *name,
				      uint8_t **data)
{
	return flashrom_read_by_name(data, HOST_FIRMWARE, name);
}

static int host_firmware_write_by_name(struct platform_intf *intf,
				       struct eeprom *eeprom,
				       const char *name,
				       unsigned int len,
				       uint8_t *data)
{
	return flashrom_write_by_name(len, data, HOST_FIRMWARE, name);
}

static struct eeprom_dev host_firmware = {
	.size		= host_firmware_size,
	.read		= host_firmware_read,
	.read_by_name	= host_firmware_read_by_name,
	.write_by_name	= host_firmware_write_by_name,
	.get_map	= eeprom_get_fmap,
};

static struct eeprom_region host_firmware_regions[] = {
	{
		.name	= "RW_NVRAM",
		.flag	= EEPROM_FLAG_VBNV,
	},
	{ 0 },
};

static struct eeprom eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		.flags		= EEPROM_FLAG_RDWR | EEPROM_FLAG_FMAP,
		.device		= &host_firmware,
		.regions	= &host_firmware_regions[0],
	},
	{ 0 },
};

struct eeprom_cb reef_eeprom_cb = {
	.eeprom_list	= eeproms,
};
