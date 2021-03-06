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

#include <limits.h>

#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/intel/nm10.h"

#include "lib/file.h"
#include "lib/spd.h"

#define MARIO_DIMM_COUNT	1

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int mario_dimm_count(struct platform_intf *intf)
{
	return MARIO_DIMM_COUNT;
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
static int mario_dimm_map(struct platform_intf *intf,
                          enum dimm_map_type type, int dimm)
{
	int ret = -1;
	static struct dimm_map {
		int node;
		int channel;
		int slot;
		int bus;
		int address;
	} mario_dimm_map[MARIO_DIMM_COUNT] = {
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
			if (mario_dimm_map[x].bus < lowest_known_bus)
				lowest_known_bus = mario_dimm_map[x].bus;
		}

		snprintf(path, sizeof(path), "%s/%s",
		         mosys_get_root_prefix(), "/sys/bus/i2c/devices");
		x = sysfs_lowest_smbus(path, NM10_SMBUS_ADAPTER);
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
		ret = mario_dimm_map[dimm].bus + bus_offset;
		break;
	case DIMM_TO_ADDRESS:
		ret = mario_dimm_map[dimm].address;
		break;
	default:
		break;
	}

	return ret;
}

static int mario_spd_read(struct platform_intf *intf,
                          int dimm, int reg, int len, uint8_t *buf)
{
	int bus;
	int address;

	bus = intf->cb->memory->dimm_map(intf, DIMM_TO_BUS, dimm);
	address = intf->cb->memory->dimm_map(intf, DIMM_TO_ADDRESS, dimm);

	return spd_read_i2c(intf, bus, address, reg, len, buf);
}

static struct memory_spd_cb mario_spd_cb = {
	.read		= mario_spd_read,
};

struct memory_cb google_cr48_memory_cb = {
	.dimm_count	= mario_dimm_count,
	.dimm_map	= mario_dimm_map,
	.spd		= &mario_spd_cb,
};
