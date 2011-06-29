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

#include <limits.h>

#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/nm10.h"

#include "intf/mmio.h"

#include "lib/file.h"
#include "lib/spd.h"
#include "lib/vpd.h"

#define ALEX_DIMM_COUNT	1

/*
 * alex_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int alex_dimm_count(struct platform_intf *intf)
{
	return ALEX_DIMM_COUNT;
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
static int alex_dimm_map(struct platform_intf *intf,
                          enum dimm_map_type type, int dimm)
{
	int ret = -1;
	static struct dimm_map {
		int node;
		int channel;
		int slot;
		int bus;
		int address;
	} alex_dimm_map[ALEX_DIMM_COUNT] = {
		/* Node 0 */
		{ 0, 0, 0, 2, 0x50 }
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

		for (x = 0; x < intf->cb->memory->dimm_count(intf); x++) {
			if (alex_dimm_map[x].bus < lowest_known_bus)
				lowest_known_bus = alex_dimm_map[x].bus;
		}

		snprintf(path, sizeof(path), "%s/%s",
		         mosys_get_root_prefix(), "/sys/bus/i2c/devices");
		x = sysfs_lowest_smbus(path, NM10_SMBUS_ADAPTER);
		if (x >= 0) {
			lprintf(LOG_DEBUG, "%s: bus_offset: %d\n",
			        __func__, bus_offset);
			bus_offset = x - lowest_known_bus;
		} else {
			lprintf(LOG_DEBUG, "%s: unable to determine "
			                   "bus offset\n", __func__);
			bus_offset = 0;
		}

		first_run = 0;
	}

	switch (type) {
	case DIMM_TO_BUS:
		ret = alex_dimm_map[dimm].bus + bus_offset;
		break;
	case DIMM_TO_ADDRESS:
		ret = alex_dimm_map[dimm].address;
		break;
	default:
		break;
	}

	return ret;
}

static int alex_spd_read_i2c(struct platform_intf *intf,
                             int dimm, int reg, int len, uint8_t *buf)
{
	int bus, address;

	bus = intf->cb->memory->dimm_map(intf, DIMM_TO_BUS, dimm);
	address = intf->cb->memory->dimm_map(intf, DIMM_TO_ADDRESS, dimm);

	return spd_read_i2c(intf, bus, address, reg, len, buf);
}

static int alex_spd_read_vpd(struct platform_intf *intf,
                             int dimm, int reg, int len, uint8_t *buf)
{
	uint8_t *spd;
	struct gpio_map *gpio = NULL;
	int level;
	int ret = -1;

	if (!intf->cb->gpio || !intf->cb->gpio->map || !intf->cb->gpio->read)
		return -1;

	/* The BOARD_CONFIG GPIO (GPIO36) toggles which SPD blob (written
	 * in the VPD area) to use. */
	if ((gpio = intf->cb->gpio->map(intf, "BOARD_CONFIG")) == NULL)
		return -1;

	level = intf->cb->gpio->read(intf, gpio);
	lprintf(LOG_DEBUG, "%s: %s level: %d\n", __func__, gpio->name, level);
	if (level == 0)
		spd = (uint8_t *)(vpd_rom_base + 0x220400);
	else if (level == 1)
		spd = (uint8_t *)(vpd_rom_base + 0x220500);
	else
		return -1;

	lprintf(LOG_DEBUG, "%s: reading %d bytes from 0x%lx\n",
	                   __func__, len, spd + reg);
	if (mmio_read(intf, spd + reg, len, buf) == 0)
		ret = len;

	return ret;
}

static int alex_spd_read(struct platform_intf *intf,
                          int dimm, int reg, int len, uint8_t *buf)
{
	int ret = 0;

	/*
	 * There is 1 DIMM in Alex, however SPD information can come from
	 * one of three sources due to possible removal of the SPD EEPROM:
	 * 1. I2C. Try this first.
	 * 2. VPD -- If GPIO36 == 0, it will use information stored in the
	 *           VPD region.
	 */
	if ((ret = alex_spd_read_i2c(intf, dimm, reg, len, buf)) == len)
		return ret;
	if ((ret = alex_spd_read_vpd(intf, dimm, reg, len, buf)) == len)
		return ret;
	return -1;
}

static struct memory_spd_cb alex_spd_cb = {
	.read		= alex_spd_read,
};

struct memory_cb alex_pinetrail_memory_cb = {
	.dimm_count	= alex_dimm_count,
	.dimm_map	= alex_dimm_map,
	.spd		= &alex_spd_cb,
};
