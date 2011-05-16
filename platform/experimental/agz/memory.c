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

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/spd.h"

#define AGZ_DIMM_COUNT	1

/*
 * agz_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int agz_dimm_count(struct platform_intf *intf)
{
	return AGZ_DIMM_COUNT;
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
static int agz_dimm_map(struct platform_intf *intf,
                          enum dimm_map_type type, int dimm)
{
	int ret = -1;
	static struct dimm_map {
		int node;
		int channel;
		int slot;
		int bus;
		int address;
	} agz_dimm_map[AGZ_DIMM_COUNT] = {
		/* Node 0 */
		{ 0, 0, 0, 2, 0x50 }
	};
	static int bus_offset = -1;
	const char smbus_i801[] = "SMBus I801 adapter";

	if (dimm < 0 || dimm >= intf->cb->memory->dimm_count(intf)) {
		lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
		return -1;
	}

	if (bus_offset == -1) {
		if (intf->op->i2c->match_bus(intf, 2, smbus_i801))
			bus_offset = 0;
		else if (intf->op->i2c->match_bus(intf, 14, smbus_i801))
			bus_offset = 12;
		else
			return -1;
	}

	switch (type) {
	case DIMM_TO_BUS:
		ret = agz_dimm_map[dimm].bus + bus_offset;
		break;
	case DIMM_TO_ADDRESS:
		ret = agz_dimm_map[dimm].address;
		break;
	default:
		break;
	}

	return ret;
}

static int agz_spd_read(struct platform_intf *intf,
                          int dimm, int reg, int len, uint8_t *buf)
{
	int bus;
	int address;

	bus = intf->cb->memory->dimm_map(intf, DIMM_TO_BUS, dimm);
	address = intf->cb->memory->dimm_map(intf, DIMM_TO_ADDRESS, dimm);

	return spd_read_i2c(intf, bus, address, reg, len, buf);
}

static struct memory_spd_cb agz_spd_cb = {
	.read		= agz_spd_read,
};

struct memory_cb agz_pinetrail_memory_cb = {
	.dimm_count	= agz_dimm_count,
	.dimm_map	= agz_dimm_map,
	.spd		= &agz_spd_cb,
};
