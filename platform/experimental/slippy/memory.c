/*
 * Copyright 2013, Google Inc.
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

#include <limits.h>	/* for INT_MAX */

#include <arpa/inet.h>	/* ntohl() */

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"

#include "lib/cbfs_core.h"
#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/math.h"
#include "lib/spd.h"

#include "slippy.h"

#define SLIPPY_DIMM_COUNT	2

/*
 * SPD blob contains up to eight entries which are selected by
 * board strappings.
 *
 * GPIO13: Bit 0
 * GPIO09: Bit 1
 * GPIO47: Bit 2
 */
static int slippy_get_spd_index(struct platform_intf *intf)
{
	int spd_index = 0;
	int val;
	struct gpio_map ram_id0 = { 13, GPIO_IN, 0, 0, 13 };
	struct gpio_map ram_id1 = {  9, GPIO_IN, 0, 0, 9 };
	struct gpio_map ram_id2 = { 47, GPIO_IN, 0, 1, 15 };

	if ((val = intf->cb->gpio->read(intf, &ram_id0)) < 0)
		return -1;
	spd_index |= val;

	if ((val = intf->cb->gpio->read(intf, &ram_id1)) < 0)
		return -1;
	spd_index |= val << 1;

	if ((val = intf->cb->gpio->read(intf, &ram_id2)) < 0)
		return -1;
	spd_index |= val << 2;

	return spd_index;
}

/*
 * SPD blob contains up to eight entries which are selected by
 * board strappings.
 *
 * GPIO67: Bit 0
 * GPIO68: Bit 1
 * GPIO69: Bit 2
 * GPIO65: Bit 3
 */
static int samus_get_spd_index(struct platform_intf *intf)
{
	int spd_index = 0;
	int val;
	struct gpio_map ram_id0 = { 67, GPIO_IN, 0, 2, 3 };
	struct gpio_map ram_id1 = { 68, GPIO_IN, 0, 2, 4 };
	struct gpio_map ram_id2 = { 69, GPIO_IN, 0, 2, 5 };
	struct gpio_map ram_id3 = { 65, GPIO_IN, 0, 2, 2 };

	if ((val = intf->cb->gpio->read(intf, &ram_id0)) < 0)
		return -1;
	spd_index |= val;

	if ((val = intf->cb->gpio->read(intf, &ram_id1)) < 0)
		return -1;
	spd_index |= val << 1;

	if ((val = intf->cb->gpio->read(intf, &ram_id2)) < 0)
		return -1;
	spd_index |= val << 2;

	if ((val = intf->cb->gpio->read(intf, &ram_id3)) < 0)
		return -1;
	spd_index |= val << 3;

	return spd_index;
}

/*
 * slippy_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int slippy_dimm_count(struct platform_intf *intf)
{
	if (!strncmp(intf->name, "Falco", 5)) {
		/* Falco has 1 or 2 DIMM config based on RAM_ID.
		 * {0,0,0} = 4GB Micron
		 * {0,0,1} = 4GB Hynix
		 * {0,1,0} = 4GB Elpida
		 * {0,1,1} = 2GB Micron
		 * {1,0,0} = 2GB Hynix
		 * {1,0,1} = 2GB Elpida
		 * {1,1,0} = 4GB Samsung
		 * {1,1,1} = 2GB Samsung
		 */
		int index = slippy_get_spd_index(intf);
		switch (index) {
		case 3: case 4: case 5: case 7:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Peppy", 5)) {
		/* Peppy has 1 or 2 DIMM config based on RAM_ID.
		 * {0,0,0} = 4GB Micron
		 * {0,0,1} = 4GB Hynix
		 * {0,1,0} = 4GB Elpida
		 * {0,1,1} = UNUSED
		 * {1,0,0} = 2GB Micron
		 * {1,0,1} = 2GB Hynix
		 * {1,1,0} = 2GB Elpida
		 */
		return slippy_get_spd_index(intf) >= 4 ? 1 : 2;
	} else if (!strncmp(intf->name, "Samus", 5)) {
		/* Samus has 2 DIMM config */
		return 2;
	} else if (!strncmp(intf->name, "Leon", 4)) {
		/* Leon RAM_ID
		 * {0,0,0} = 4G Micron
		 * {0,0,1} = 4G Hynix
		 * {0,1,0} = 4G Samsung
		 * {1,0,1} = 2G Hynix
		 * {1,1,0} = 2G Samsung
		 */
		return slippy_get_spd_index(intf) >= 4 ? 1 : 2;
	} else if (!strncmp(intf->name, "Wolf", 4)) {
		/* Wolf RAM_ID
		 * {0,0,0} = 4G Micron
		 * {0,0,1} = 4G Hynix
		 * {0,1,0} = 4G Samsung
		 * {0,1,1} = 2G Micron
		 * {1,0,0} = 2G Hynix
		 * {1,0,1} = 2G Samsung
		 */
		return slippy_get_spd_index(intf) >= 3 ? 1 : 2;
	}
	else
		return SLIPPY_DIMM_COUNT;
}

static int slippy_spd_read_cbfs(struct platform_intf *intf,
				int dimm, int reg, int len, uint8_t *buf)
{
	static int first_run = 1;
	static uint8_t *bootblock = NULL;
	size_t size = SLIPPY_HOST_FIRMWARE_ROM_SIZE;
	struct cbfs_file *file;
	int spd_index = 0;
	uint32_t spd_offset;

	if (dimm > slippy_dimm_count(intf)) {
		lprintf(LOG_DEBUG, "%s: Invalid DIMM specified\n", __func__);
		return -1;
	}

	if (first_run) {
		bootblock = mosys_malloc(size);	/* FIXME: overkill */
		add_destroy_callback(free, bootblock);
		first_run = 0;

		/* read SPD from CBFS entry located within bootblock region */
		if (flashrom_read(bootblock, size,
				  INTERNAL_BUS_SPI, "BOOT_STUB") < 0)
			return -1;
	}

	if ((file = cbfs_find("spd.bin", bootblock, size)) == NULL)
		return -1;

	/* Special case for Samus */
	if (!strncmp(intf->name, "Samus", 5))
		spd_index = samus_get_spd_index(intf);
	else
		spd_index = slippy_get_spd_index(intf);
	if (spd_index < 0)
		return -1;

	spd_offset = ntohl(file->offset) + (spd_index * 256);
	lprintf(LOG_DEBUG, "Using memory config %u\n", spd_index);
	memcpy(buf, (void *)file + spd_offset + reg, len);

	return len;
}

static int slippy_spd_read(struct platform_intf *intf,
			 int dimm, int reg, int len, uint8_t *buf)
{
	return slippy_spd_read_cbfs(intf, dimm, reg, len, buf);
}

static struct memory_spd_cb slippy_spd_cb = {
	.read		= slippy_spd_read,
};

struct memory_cb slippy_memory_cb = {
	.dimm_count	= slippy_dimm_count,
	.spd		= &slippy_spd_cb,
};
