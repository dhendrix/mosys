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
	CMOS_DEVICE_NM10,
};

#if 0
struct valstr standard_rtc_vars[] = {
	/* 0x00 - 0x0d: RTC */
	{ 0x00, "seconds" },
	{ 0x01, "seconds_alarm" },
	{ 0x02, "minutes" },
	{ 0x03, "minutes_alarm" },
	{ 0x04, "hours" },
	{ 0x05, "hours_alarm" },
	{ 0x06, "day_of_week" },
	{ 0x07, "day_of_month" },
	{ 0x08, "month" },
	{ 0x09, "year" },
	{ 0x0a, "status_a" },
	{ 0x0b, "status_b" },
	{ 0x0c, "status_c" },
	{ 0x0d, "status_d" },
};
#endif

#if 0
static struct valstr acer_chromia700_cmos_standard_vars[] = {
	/* 0x10 - 0x3f: standard cmos config space */
	{ 0x2e, "csum_high" },
	{ 0x2f, "csum_low" },
	{ 0x32, "century", },
	{ 0x34, "timezone_high" },	/* are high and low reversed? */
	{ 0x35, "timezone_low" },
	{ 0x36, "daylight_savings" },

	{ 0x00, NULL },
};
#endif

static struct valstr acer_chromia700_cmos_oem_vars[] = {
	/* this is the subset of stuff we are usually interested in */
	{ 0x6c, "boot_mode" },

	{ 0x00, NULL },
};

struct cmos_map {
	enum cmos_device type;
	const char *device;
	int bank;
	int length;
	int clear_start;	/* first bytes are usually reserved for RTC */
	struct valstr *var_list;
};

struct cmos_map acer_chromia700_cmos_map[] = {
	{ CMOS_DEVICE_NM10, "NM10", 0, 128, 0x0e, acer_chromia700_cmos_oem_vars },
};

static const uint16_t nm10_cmos_port[] = { 0x70 };

static uint8_t nm10_read_cmos(struct platform_intf *intf,
			      int addr, int reg)
{
	uint8_t data;
	io_write8(intf, nm10_cmos_port[addr], reg);
	io_read8(intf, nm10_cmos_port[addr] + 1, &data);
	return data;
}

static void nm10_write_cmos(struct platform_intf *intf,
			       int addr, int reg, uint8_t val)
{
	io_write8(intf, nm10_cmos_port[addr], reg);
	io_write8(intf, nm10_cmos_port[addr] + 1, val);
}

static int acer_chromia700_nvram_list(struct platform_intf *intf)
{
	struct valstr *vs;
	struct cmos_map *map = acer_chromia700_cmos_map;
	struct kv_pair *kv = kv_pair_new();
	int rc;

	/* handle each cmos bank */
	for (vs = map->var_list; vs->str; vs++) {
		uint8_t val;

		switch (map->type) {
		case CMOS_DEVICE_NM10:
			val = nm10_read_cmos(intf, map->bank, vs->val);
			break;
		}

		kv_pair_fmt(kv, "device", "%s", map->device);
		kv_pair_fmt(kv, "name", "%s", vs->str);
		kv_pair_fmt(kv, "value", "0x%02x", val);
	}

	rc = kv_pair_print(kv);
	kv_pair_free(kv);
	return rc;
}

static int acer_chromia700_nvram_dump(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;
	uint8_t cmos_data[128];

	/* handle each cmos bank */
	for (dev = 0;
	     dev < sizeof(acer_chromia700_cmos_map) /
	           sizeof(acer_chromia700_cmos_map[0]);
	     dev++) {
		map = &acer_chromia700_cmos_map[dev];

		if (map->length > sizeof(cmos_data))
			continue;
		memset(cmos_data, 0, sizeof(cmos_data));

		mosys_printf("%s CMOS Bank %d (%d bytes)\n",
		       map->device, map->bank, map->length);

		switch (map->type) {
		case CMOS_DEVICE_NM10:
			for (off = 0; off < map->length; off++)
				cmos_data[off] = nm10_read_cmos(
					intf, map->bank, off);
			break;
		}

		print_buffer(cmos_data, map->length);
		mosys_printf("\n");
	}

	return 0;
}

#if 0
/*
 * FIXME: The "nvram clear" function was removed because this particular
 * version is unsafe. We can re-add it once we are certain that the command
 * will not interfere with the factory flow if invoked.
 * Details @ chrome-os-partner:7343
 */
static int acer_chromia700_nvram_clear(struct platform_intf *intf)
{
	struct cmos_map *map;
	int off, dev;

	/* handle each cmos bank */
	for (dev = 0;
	     dev < (sizeof(acer_chromia700_cmos_map) / sizeof(struct cmos_map));
	     dev++) {
		map = &acer_chromia700_cmos_map[dev];

		switch (map->type) {
		case CMOS_DEVICE_NM10:
			for (off = map->clear_start; off < map->length; off++) {
				/* don't clear century byte */
				if (!map->bank && off == 0x32)
					continue;
				nm10_write_cmos(intf, map->bank, off, 0xff);
			}
			break;
		}
	}

	return 0;
}
#endif

struct nvram_cb acer_chromia700_nvram_cb = {
	.list	= acer_chromia700_nvram_list,
	.dump	= acer_chromia700_nvram_dump,
//	.clear	= acer_chromia700_nvram_clear,
};
