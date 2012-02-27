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

#include <limits.h>	/* for INT_MAX */

#include <arpa/inet.h>	/* ntohl() */

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/series6.h"

#include "lib/cbfs_core.h"
#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/math.h"
#include "lib/spd.h"

#include "lumpy.h"

#define LUMPY_DIMM_COUNT	2

/*
 * lumpy_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int lumpy_dimm_count(struct platform_intf *intf)
{
	return LUMPY_DIMM_COUNT;
}

/*
 * dimm_map  -  Convert logical dimm number to useful values
 *
 * @intf:       platform interface
 * @dimm:       logical dimm number
 * @type:       conversion type
 *
 * returns specified converted value
 * returns <0 to indicate error
 */
static int lumpy_i2c_dimm_map(struct platform_intf *intf,
                              enum dimm_map_type type, int dimm)
{
	int ret = -1;
	/* Lumpy can have two DIMMs, but only one will be on I2C */
	static struct dimm_map {
		int node;
		int channel;
		int slot;
		int bus;
		int address;
	} lumpy_dimm_map[] = {
		/* Node 0 */
		{ 0, 0, 0, 15, 0x50 }
	};
	static unsigned int first_run = 1;
	static int bus_offset = 0;

	if (dimm < 0 || dimm >= intf->cb->memory->dimm_count(intf)) {
		lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
		return -1;
	}

	/*
	 * Determine offset for smbus numbering:
	 * 1. Scan known bus numbers for lowest value.
	 * 2. Scan /sys for SMBus entries that match the adapter name.
	 * 3. Calculate the difference between the lowest expected bus number
	 *    and the lowest bus number seen in sysfs matching the criteria.
	 */
	if (first_run) {
		char path[PATH_MAX];
		int lowest_known_bus = INT_MAX, x;

		if (lumpy_dimm_map[0].bus < lowest_known_bus)
			lowest_known_bus = lumpy_dimm_map[0].bus;

		snprintf(path, sizeof(path), "%s/%s",
		         mosys_get_root_prefix(), "/sys/bus/i2c/devices");
		x = sysfs_lowest_smbus(path, SERIES6_SMBUS_ADAPTER);
		if (x >= 0) {
			bus_offset = x - lowest_known_bus;
			lprintf(LOG_DEBUG, "%s: bus_offset: %d\n",
			        __func__, bus_offset);
		} else {
			lprintf(LOG_DEBUG, "%s: unable to determine "
			                   "bus offset\n", __func__);
			bus_offset = 0;
		}

		first_run = 0;
	}

	switch (type) {
	case DIMM_TO_BUS:
		ret = lumpy_dimm_map[0].bus + bus_offset;
		break;
	case DIMM_TO_ADDRESS:
		ret = lumpy_dimm_map[0].address;
		break;
	default:
		break;
	}

	return ret;
}

static int lumpy_spd_read_cbfs(struct platform_intf *intf,
                               int dimm, int reg, int len, uint8_t *buf)
{
	int rc = -1;
	static int first_run = 1;
	static uint8_t *bootblock = NULL;
	size_t size = LUMPY_HOST_FIRMWARE_ROM_SIZE;
	struct cbfs_file *file;
	uint8_t spd_index = 0, straps = 0;
	uint32_t spd_offset;

	if (first_run == 1) {
		bootblock = mosys_malloc(size);	/* FIXME: overkill */
		add_destroy_callback(free, bootblock);
		first_run = 0;

		/* read SPD from CBFS entry located within bootblock region */
		if (flashrom_read(bootblock, size,
		                  INTERNAL_BUS_SPI, "BOOT_STUB") < 0)
			goto lumpy_spd_read_cbfs_exit;
	}

	if ((file = cbfs_find("spd.bin", bootblock, size)) == NULL)
		goto lumpy_spd_read_cbfs_exit;

	/*
	 * SPD blob contains up to six entries which are selected by
	 * board strappings.
	 *
	 * GPIO33: Capacity
	 * GPIO41: Die revision
	 * GPIO49: Board revision
	 */
	{
		int val;
		struct gpio_map gpio33 = { 33, GPIO_IN, LUMPY_GPIO_PCH, 1,  1 };
		struct gpio_map gpio41 = { 41, GPIO_IN, LUMPY_GPIO_PCH, 1,  9 };
		struct gpio_map gpio49 = { 49, GPIO_IN, LUMPY_GPIO_PCH, 1, 17 };

		if ((val = intf->cb->gpio->read(intf, &gpio33)) < 0)
			goto lumpy_spd_read_cbfs_exit;
		straps |= val;

		if ((val = intf->cb->gpio->read(intf, &gpio41)) < 0)
			goto lumpy_spd_read_cbfs_exit;
		straps |= val << 1;

		if ((val = intf->cb->gpio->read(intf, &gpio49)) < 0)
			goto lumpy_spd_read_cbfs_exit;
		straps |= val << 2;
	}

	switch (straps) {
	case 0:
		spd_index = 0;
		break;
	case 2:
		spd_index = 1;
		break;
	case 1:
	case 3:
		spd_index = 2;
		break;
	case 4:
		spd_index = 3;
		break;
	case 6:
		spd_index = 4;
		break;
	case 5:
	case 7:
		spd_index = 5;
		break;
	default:
		lprintf(LOG_DEBUG, "Unknown memory config\n");
		goto lumpy_spd_read_cbfs_exit;
	}

	spd_offset = ntohl(file->offset) + (spd_index * 256);
	lprintf(LOG_DEBUG, "Using memory config %u\n", straps);
	memcpy(buf, (void *)file + spd_offset + reg, len);
	rc = len;
lumpy_spd_read_cbfs_exit:
	return rc;
}

static int lumpy_spd_read_i2c(struct platform_intf *intf,
                              int dimm, int reg, int len, uint8_t *buf)
{
	int bus;
	int address;

	/* read SPD from slotted DIMM if available */
	bus = intf->cb->memory->dimm_map(intf, DIMM_TO_BUS, dimm);
	address = intf->cb->memory->dimm_map(intf, DIMM_TO_ADDRESS, dimm);
	return spd_read_i2c(intf, bus, address, reg, len, buf);
}

static int lumpy_spd_read(struct platform_intf *intf,
                          int dimm, int reg, int len, uint8_t *buf)
{
	int ret = -1;

	/*
	 * Lumpy has two memory modules: One on-board, and an optional slotted
	 * DIMM. We'll call the on-board module "dimm 0" and the optional DIMM
	 * "dimm 1".
	 */
	if (dimm == 0) {
		if (intf->cb->sys && intf->cb->sys->firmware_vendor) {
			const char *bios = intf->cb->sys->firmware_vendor(intf);
			if (bios && !strcasecmp(bios, "coreboot"))
				ret = lumpy_spd_read_cbfs(intf, dimm,
				                          reg, len, buf);
			free((void *)bios);
		}
	} else if (dimm == 1)
		ret = lumpy_spd_read_i2c(intf, dimm, reg, len, buf);
	else
		lprintf(LOG_DEBUG, "%s: Invalid DIMM: %d\n", __func__, dimm);

	return ret;
}

static struct memory_spd_cb lumpy_spd_cb = {
	.read		= lumpy_spd_read,
};

struct memory_cb lumpy_memory_cb = {
	.dimm_count	= lumpy_dimm_count,
	.dimm_map	= lumpy_i2c_dimm_map,
	.spd		= &lumpy_spd_cb,
};
