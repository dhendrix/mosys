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

struct cmos_var_map {
	uint8_t offset;
	uint8_t length;
	char *desc;
};

struct cmos_map {
	enum cmos_device type;
	const char *device;
	int bank;
	int length;
	int clear_start;	/* first bytes are usually reserved for RTC */
	struct cmos_var_map *var_list;
};

static struct cmos_var_map coreboot_cmos_bank0_vars[] = {
	{ 0x70, 1, "Post Code Bank" },
	{ 0x71, 1, "Post Code Bank 0" },
	{ 0x72, 1, "Post Code Bank 1" },
	{ 0x73, 4, "Post Extra Bank 0" },
	{ 0x77, 4, "Post Extra Bank 1" },
	{ 0 }
};

static struct cmos_var_map coreboot_cmos_bank1_vars[] = {
	{ 0x12, 4, "Boot Count" }, /* 0x92 */
	{ 0 }
};

struct cmos_map cyan_cmos_map[] = {
	{ CMOS_DEVICE_PCH, "LPSS0", 0, 128, 0x29, coreboot_cmos_bank0_vars },
	{ CMOS_DEVICE_PCH, "LPSS1", 1, 128, 0x00, coreboot_cmos_bank1_vars },
};

static const uint16_t cyan_cmos_port[] = { 0x70, 0x72 };

static uint8_t cyan_read_cmos(struct platform_intf *intf,
                                int addr, int reg)
{
	uint8_t data;
	io_write8(intf, cyan_cmos_port[addr], reg);
	io_read8(intf, cyan_cmos_port[addr] + 1, &data);
	return data;
}

static void cyan_write_cmos(struct platform_intf *intf,
			       int addr, int reg, uint8_t val)
{
	io_write8(intf, cyan_cmos_port[addr], reg);
	io_write8(intf, cyan_cmos_port[addr] + 1, val);
}

static int cyan_nvram_list_bank(struct platform_intf *intf,
				  struct cmos_map *map)
{
	struct cmos_var_map *var;
	int i;

	/* handle each cmos bank */
	for (var = map->var_list; var && var->desc; var++) {
		struct kv_pair *kv = kv_pair_new();
		uint32_t val = 0;

		switch (map->type) {
		case CMOS_DEVICE_PCH:
			for (i = 0; i < var->length; i++)
				val |= cyan_read_cmos(
					intf, map->bank,
					var->offset + i) << (i*8);
			break;
		}

		kv_pair_add(kv, "device", map->device);
		kv_pair_add(kv, "name", var->desc);
		kv_pair_fmt(kv, "value", "0x%x", val);
		kv_pair_print(kv);
		kv_pair_free(kv);
	}

	return 0;
}

static int cyan_nvram_list(struct platform_intf *intf)
{
	int dev, rc = 0;

	/* handle each cmos bank */
	for (dev = 0; dev < ARRAY_SIZE(cyan_cmos_map); dev++)
		rc |= cyan_nvram_list_bank(intf, &cyan_cmos_map[dev]);

	return rc;
}

static int cyan_nvram_dump(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;
	uint8_t cmos_data[128];

	/* handle each cmos bank */
	for (dev = 0; dev < ARRAY_SIZE(cyan_cmos_map); dev++) {
		map = &cyan_cmos_map[dev];

		if (map->length > sizeof(cmos_data))
			continue;
		memset(cmos_data, 0, sizeof(cmos_data));

		mosys_printf("%s CMOS Bank %d (%d bytes)\n",
		       map->device, map->bank, map->length);

		switch (map->type) {
		case CMOS_DEVICE_PCH:
			for (off = 0; off < map->length; off++)
				cmos_data[off] = cyan_read_cmos(
					intf, map->bank, off);
			break;
		}

		print_buffer(cmos_data, map->length);
		mosys_printf("\n");
	}

	return 0;
}

static int cyan_nvram_clear(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;

	/* handle each cmos bank */
	for (dev = 0;
	     dev < (sizeof(cyan_cmos_map) / sizeof(struct cmos_map));
	     dev++) {
		map = &cyan_cmos_map[dev];

		switch (map->type) {
		case CMOS_DEVICE_PCH:
			for (off = map->clear_start; off < map->length; off++)
				cyan_write_cmos(intf, map->bank, off, 0x00);
			break;
		}
	}

	return 0;
}

struct nvram_cb cyan_nvram_cb = {
	.list	= cyan_nvram_list,
	.dump	= cyan_nvram_dump,
	.clear	= cyan_nvram_clear,
};
