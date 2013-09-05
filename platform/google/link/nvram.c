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
	CMOS_DEVICE_PCH,
};

struct cmos_map {
	enum cmos_device type;
	const char *device;
	int bank;
	int length;
	int clear_start;	/* first bytes are usually reserved for RTC */
	struct valstr *var_list;
};

struct cmos_map link_cmos_map[] = {
	{ CMOS_DEVICE_PCH, "6-Series", 0, 128, 0x29, NULL },
};

static const uint16_t link_cmos_port[] = { 0x70 };

static uint8_t link_read_cmos(struct platform_intf *intf,
                                int addr, int reg)
{
	uint8_t data;
	io_write8(intf, link_cmos_port[addr], reg);
	io_read8(intf, link_cmos_port[addr] + 1, &data);
	return data;
}

static void link_write_cmos(struct platform_intf *intf,
			       int addr, int reg, uint8_t val)
{
	io_write8(intf, link_cmos_port[addr], reg);
	io_write8(intf, link_cmos_port[addr] + 1, val);
}

static int link_nvram_dump(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;
	uint8_t cmos_data[128];

	/* handle each cmos bank */
	for (dev = 0;
	     dev < sizeof(link_cmos_map) /
	           sizeof(link_cmos_map[0]);
	     dev++) {
		map = &link_cmos_map[dev];

		if (map->length > sizeof(cmos_data))
			continue;
		memset(cmos_data, 0, sizeof(cmos_data));

		mosys_printf("%s CMOS Bank %d (%d bytes)\n",
		       map->device, map->bank, map->length);

		switch (map->type) {
		case CMOS_DEVICE_PCH:
			for (off = 0; off < map->length; off++)
				cmos_data[off] = link_read_cmos(
					intf, map->bank, off);
			break;
		}

		print_buffer(cmos_data, map->length);
		mosys_printf("\n");
	}

	return 0;
}

static int link_nvram_clear(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;

	/* handle each cmos bank */
	for (dev = 0;
	     dev < (sizeof(link_cmos_map) / sizeof(struct cmos_map));
	     dev++) {
		map = &link_cmos_map[dev];

		switch (map->type) {
		case CMOS_DEVICE_PCH:
			for (off = map->clear_start; off < map->length; off++)
				link_write_cmos(intf, map->bank, off, 0x00);
			break;
		}
	}

	return 0;
}

struct nvram_cb link_nvram_cb = {
	.dump	= link_nvram_dump,
	.clear	= link_nvram_clear,
};
