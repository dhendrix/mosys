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

#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/series6.h"

#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/spd.h"
#include "lib/smbios.h"
#include "lib/smbios_tables.h"
#include "mosys/kv_pair.h"

#include "auron.h"

#define AURON_DIMM_COUNT	2

/*
 * auron_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int auron_dimm_count(struct platform_intf *intf)
{
	int status = 0, dimm_cnt = 0;
	struct smbios_table table;

	while (status == 0) {
		status = smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm_cnt,
					   &table,
					   SMBIOS_LEGACY_ENTRY_BASE,
					   SMBIOS_LEGACY_ENTRY_LEN);
		if(status == 0)
			dimm_cnt++;
	}
	return dimm_cnt;
}

static int auron_spd_read(struct platform_intf *intf,
		 int dimm, int reg, int spd_len, uint8_t *spd_buf)
{
	static uint8_t *fw_buf;
	static int fw_size = 0;

	/* dimm cnt is 0 based */
	if (dimm >= intf->cb->memory->dimm_count(intf)) {
		lprintf(LOG_DEBUG, "%s: Invalid DIMM specified\n", __func__);
		return -1;
	}

	if (fw_size < 0)
		return -1;	/* previous attempt failed */

	if (!fw_size) {
		fw_size = flashrom_read_host_firmware_region(intf, &fw_buf);
		if (fw_size < 0)
			return -1;
		add_destroy_callback(free, fw_buf);
	}

	return spd_read_from_cbfs(intf, dimm, reg,
				spd_len, spd_buf, fw_size, fw_buf);
}

int auron_dimm_speed(struct platform_intf *intf,
		     int dimm, struct kv_pair *kv)
{
	struct smbios_table table;
	char speed[10];
	if (smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		return -1;
	}
	sprintf(speed, "%d", table.data.mem_device.speed);
	kv_pair_add(kv, "speed: ", speed);

	return 0;
}

/*
 * dimm_auron_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_auron_dimm_count(struct platform_intf *intf)
{
	return AURON_DIMM_COUNT;
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
static int dimm_auron_dimm_map(struct platform_intf *intf,
                          enum dimm_map_type type, int dimm)
{
	int ret = -1;
	static struct dimm_map {
		int node;
		int channel;
		int slot;
		int bus;
		int address;
	} auron_dimm_map[AURON_DIMM_COUNT] = {
		/* Node 0 */
		{ 0, 0, 0, 17, 0x50 },
		{ 0, 0, 0, 17, 0x52 }
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
			if (auron_dimm_map[x].bus < lowest_known_bus)
				lowest_known_bus = auron_dimm_map[x].bus;
		}

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
		ret = auron_dimm_map[dimm].bus + bus_offset;
		break;
	case DIMM_TO_ADDRESS:
		ret = auron_dimm_map[dimm].address;
		break;
	default:
		break;
	}

	return ret;
}

static int dimm_auron_spd_read(struct platform_intf *intf,
                          int dimm, int reg, int len, uint8_t *buf)
{
	int bus;
	int address;

	bus = intf->cb->memory->dimm_map(intf, DIMM_TO_BUS, dimm);
	address = intf->cb->memory->dimm_map(intf, DIMM_TO_ADDRESS, dimm);

	return spd_read_i2c(intf, bus, address, reg, len, buf);
}

static struct memory_spd_cb auron_spd_cb = {
	.read		= auron_spd_read,
};

static struct memory_spd_cb dimm_auron_spd_cb = {
	.read		= dimm_auron_spd_read,
};

struct memory_cb auron_memory_cb = {
	.dimm_count	= auron_dimm_count,
	.spd		= &auron_spd_cb,
	.dimm_speed	= &auron_dimm_speed,
};

struct memory_cb dimm_memory_cb = {
	.dimm_count	= dimm_auron_dimm_count,
	.dimm_map	= dimm_auron_dimm_map,
	.spd		= &dimm_auron_spd_cb,
};
