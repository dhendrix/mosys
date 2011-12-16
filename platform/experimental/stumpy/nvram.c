/*
 * Copyright (C) 2011 Google Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <valstr.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "mosys/output.h"

#include "intf/io.h"

#include "lib/math.h"

enum cmos_device {
	CMOS_DEVICE_SERIES6,
};

struct cmos_map {
	enum cmos_device type;
	const char *device;
	int bank;
	int length;
	int clear_start;	/* first bytes are usually reserved for RTC */
	struct valstr *var_list;
};

struct cmos_map stumpy_cmos_map[] = {
	{ CMOS_DEVICE_SERIES6, "6-Series", 0, 128, 0x29, NULL },
};

static const uint16_t stumpy_cmos_port[] = { 0x70 };

static uint8_t stumpy_read_cmos(struct platform_intf *intf,
                                int addr, int reg)
{
	uint8_t data;
	io_write8(intf, stumpy_cmos_port[addr], reg);
	io_read8(intf, stumpy_cmos_port[addr] + 1, &data);
	return data;
}

static void stumpy_write_cmos(struct platform_intf *intf,
			       int addr, int reg, uint8_t val)
{
	io_write8(intf, stumpy_cmos_port[addr], reg);
	io_write8(intf, stumpy_cmos_port[addr] + 1, val);
}

static int stumpy_nvram_dump(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;
	uint8_t cmos_data[128];

	/* handle each cmos bank */
	for (dev = 0;
	     dev < sizeof(stumpy_cmos_map) /
	           sizeof(stumpy_cmos_map[0]);
	     dev++) {
		map = &stumpy_cmos_map[dev];

		if (map->length > sizeof(cmos_data))
			continue;
		memset(cmos_data, 0, sizeof(cmos_data));

		mosys_printf("%s CMOS Bank %d (%d bytes)\n",
		       map->device, map->bank, map->length);

		switch (map->type) {
		case CMOS_DEVICE_SERIES6:
			for (off = 0; off < map->length; off++)
				cmos_data[off] = stumpy_read_cmos(
					intf, map->bank, off);
			break;
		}

		print_buffer(cmos_data, map->length);
		mosys_printf("\n");
	}

	return 0;
}

static int stumpy_nvram_clear(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;

	/* handle each cmos bank */
	for (dev = 0;
	     dev < (sizeof(stumpy_cmos_map) / sizeof(struct cmos_map));
	     dev++) {
		map = &stumpy_cmos_map[dev];

		switch (map->type) {
		case CMOS_DEVICE_SERIES6:
			for (off = map->clear_start; off < map->length; off++)
				stumpy_write_cmos(intf, map->bank, off, 0x00);
			break;
		}
	}

	return 0;
}

struct nvram_cb stumpy_nvram_cb = {
	.dump	= stumpy_nvram_dump,
	.clear	= stumpy_nvram_clear,
};
