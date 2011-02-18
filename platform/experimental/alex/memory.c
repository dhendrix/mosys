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
#include <uuid/uuid.h>

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

	if (dimm < 0 || dimm >= intf->cb->memory->dimm_count(intf)) {
		lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
		return -1;
	}

	switch (type) {
	case DIMM_TO_BUS:
		ret = alex_dimm_map[dimm].bus;
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
	int i, ret = 0;
	uint8_t *tmp;
	struct vpd_table table;
	struct vpd_table_binary_blob_pointer *bbp;
	uuid_t spd_uuid;

	/* FIXME: Put SPD UUID in one of the VPD headers. */
	if (uuid_parse("75f4926b-9e43-4b32-8979-eb20c0eda76a", spd_uuid) < 0)
		return -1;

	/* Note: There is only one DIMM in this machine, so we only need to
	 * find the first binary blob pointer with the SPD type */
	/* FIXME: 2 is arbitrary. We can probably do better... */
	for (i = 0; i < 2; i++) {
		if (vpd_find_table(intf, VPD_TYPE_BINARY_BLOB_POINTER, i,
		                   &table, vpd_rom_base, vpd_rom_size) < 0)
			continue;

		bbp = &table.data.blob;
		if (memcmp(bbp->uuid, spd_uuid, sizeof(spd_uuid)))
			continue;

		if (vpd_get_blob(intf, bbp, &tmp) > 0) {
			memcpy(buf + reg, tmp, len);
			ret = len;
			break;
		}
	}

	if (ret <= 0) {
		lprintf(LOG_DEBUG, "%s: Cannot find SPD\n", __func__);
		return -1;
	}

	return ret;
}

static int alex_spd_read(struct platform_intf *intf,
                          int dimm, int reg, int len, uint8_t *buf)
{
	int ret = 0;

	/* Try VPD first, then try I2C */
	if ((ret = alex_spd_read_vpd(intf, dimm, reg, len, buf)) == len)
		return ret;
	if ((ret = alex_spd_read_i2c(intf, dimm, reg, len, buf)) == len)
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
