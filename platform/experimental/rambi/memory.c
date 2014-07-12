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

#include <limits.h>	/* for INT_MAX */

#include <arpa/inet.h>	/* ntohl() */

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/globals.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/baytrail.h"

#include "lib/cbfs_core.h"
#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/math.h"
#include "lib/spd.h"

#include "rambi.h"

#define RAMBI_DIMM_COUNT	2
#define RAMBI_DIMM_SPEED	1333

/*
 * Return the number of RAM_ID strap GPIOs. These GPIOs are standardized
 * across all Rambi-class platforms, but some platforms have four GPIOs
 * instead of three because they must support > 8 DRAM SKUs.
 */
static int rambi_ramid_gpio_count(struct platform_intf *intf)
{
	int gpio_count;

	if (!strncmp(intf->name, "Glimmer", 7))
		gpio_count = 4;
	else
		gpio_count = 3;

	return gpio_count;
}

/*
 * SPD blob contains up to eight entries which are selected by
 * board strappings.
 */
static int rambi_get_spd_index(struct platform_intf *intf)
{
	int spd_index = 0;
	int gpio_count = rambi_ramid_gpio_count(intf);
	int val;
	int i;
	struct gpio_map ram_id[] = { { 37, GPIO_IN, 0, BAYTRAIL_GPSSUS_PORT },
				     { 38, GPIO_IN, 0, BAYTRAIL_GPSSUS_PORT },
				     { 39, GPIO_IN, 0, BAYTRAIL_GPSSUS_PORT },
				     { 40, GPIO_IN, 0, BAYTRAIL_GPSSUS_PORT } };

	for (i = 0; i < gpio_count; i++) {
		if ((val = intf->cb->gpio->read(intf, &ram_id[i])) < 0)
			return -1;
		spd_index |= val << i;
	}

	return spd_index;
}

/*
 * rambi_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int rambi_dimm_count(struct platform_intf *intf)
{
	/*
	 * TODO(shawnn): Find a programatic way to do this detection.
	 * Platforms have 1 or 2 DIMM config based on RAM_ID.
	 */
	if (!strncmp(intf->name, "Clapper", 7)) {
		/*
		 * {0,0,0} = 2 x 2GiB Micron
		 * {0,0,1} = 2 x 2GiB Hynix
		 * {0,1,0} = 2 x 1GiB Micron
		 * {0,1,1} = 2 x 1GiB Hynix
		 * {1,0,0} = 1 x 2GiB Micron
		 * {1,0,1} = 1 x 2GiB Hynix
		 * {1,1,0} = 2 x 2GiB Samsung
		 * {1,1,1} = 1 x 2GiB Samsung
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 4: case 5: case 7:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Enguarde", 8) ||
		   !strncmp(intf->name, "Rambi", 5)) {
		/*
		 * {0,0,0} = 2 x 2GiB Micron
		 * {0,0,1} = 2 x 2GiB Hynix
		 * {0,1,0} = 2 x 1GiB Micron
		 * {0,1,1} = 2 x 1GiB Hynix
		 * {1,0,0} = 1 x 2GiB Micron
		 * {1,0,1} = 1 x 2GiB Hynix
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 4: case 5:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Glimmer", 7)) {
		/*
		 * {0,0,0,0} = 2 x 2GiB Micron
		 * {0,0,0,1} = 2 x 2GiB Hynix
		 * {0,0,1,0} = 2 x 1GiB Micron
		 * {0,0,1,1} = 2 x 1GiB Hynix
		 * {0,1,0,0} = 2 x 1GiB Samsung
		 * {0,1,0,1} = 1 x 2GiB Hynix
		 * {0,1,1,0} = 2 x 2GiB Samsung
		 * {0,1,1,1} = 2 x 2GiB Elpida
		 * {1,0,0,0} = 1 x 2GiB Micron
		 * {1,0,0,1} = 1 x 2GiB Elpida
		 * {1,0,1,0} = 1 x 2GiB Samsung
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 5: case 8: case 9: case 10:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Gnawty", 6)) {
		/*
		 * {0,0,0} = 2 x 2GiB Micron
		 * {0,0,1} = 2 x 2GiB Hynix
		 * {0,1,0} = 2 x 1GiB Micron
		 * {0,1,1} = 1 x 2GiB Hynix
		 * {1,0,0} = 1 x 2GiB Micron
		 * {1,0,1} = 1 x 2GiB Hynix
		 * {1,1,0} = 2 x 2GiB Kingston
		 * {1,1,1} = 2 x 2GiB Hynix
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 3: case 4: case 5:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Kip", 3)) {
		/*
		 * {0,0,0} = 2 x 2GiB Micron
		 * {0,0,1} = 2 x 2GiB Hynix
		 * {0,1,0} = 2 x 2GiB Elpida
		 * {0,1,1} = 2 x 2GiB Samsung
		 * {1,0,0} = 1 x 2GiB Micron
		 * {1,0,1} = 1 x 2GiB Hynix
		 * {1,1,0} = 1 x 2GiB Elpida
		 * {1,1,1} = 1 x 2GiB Samsung
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 4: case 5: case 6: case 7:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Squawks", 7) ||
		   !strncmp(intf->name, "Quawks", 6)) {
		/*
		 * {0,0,0} = 2 x 2GiB Elpida
		 * {0,0,1} = 2 x 2GiB Hynix
		 * {0,1,0} = 2 x 1GiB Micron
		 * {0,1,1} = 2 x 2GiB Hynix
		 * {1,0,0} = 1 x 2GiB Elpida
		 * {1,0,1} = 1 x 2GiB Hynix
		 * {1,1,0} = 1 x 2GiB Hynix
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 4: case 5: case 6:
			return 1;
		default:
			return 2;
		}
	} else if (!strncmp(intf->name, "Swanky", 6)) {
		/*
		 * {0,0,0} = 1 x 2GiB Samsung
		 * {0,0,1} = 1 x 2GiB Hynix
		 * {0,1,0} = 2 x 2GiB Samsung
		 * {0,1,1} = 2 x 2GiB Hynix
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 2: case 3:
			return 2;
		default:
			return 1;
		}
	} else if (!strncmp(intf->name, "Winky", 5)) {
		/*
		 * {0,0,0} = 2 x 2GiB Micron
		 * {0,0,1} = 2 x 2GiB Samsung
		 * {1,0,0} = 1 x 2GiB Micron
		 * {1,0,1} = 1 x 2GiB Samsung
		 */
		int index = rambi_get_spd_index(intf);
		switch (index) {
		case 4: case 5: case 6: case 7:
			return 1;
		default:
			return 2;
		}
	}
	return RAMBI_DIMM_COUNT;
}

/*
 * rambi_dimm_speed - Write actual DDR speed in MHz to kv
 *
 * @intf:	platform interface
 * @dimm:	DIMM number
 * @kv:		kv_pair structure
 *
 * returns actual DDR speed in MHz
 */
static int rambi_dimm_speed(struct platform_intf *intf,
			    int dimm,
			    struct kv_pair *kv) {
	int speed = RAMBI_DIMM_SPEED;
	if (kv)
		kv_pair_fmt(kv, "speed", "%d MHz", speed);
	return speed;
}

static int rambi_spd_read_cbfs(struct platform_intf *intf,
				int dimm, int reg, int len, uint8_t *buf)
{
	static int first_run = 1;
	static uint8_t *bootblock = NULL;
	size_t size = RAMBI_HOST_FIRMWARE_ROM_SIZE;
	struct cbfs_file *file;
	int spd_index = 0;
	uint32_t spd_offset;

	if (dimm > rambi_dimm_count(intf)) {
		lprintf(LOG_DEBUG, "%s: Invalid DIMM specified\n", __func__);
		return -1;
	}

	if (first_run) {
		bootblock = mosys_malloc(size);	/* FIXME: overkill */
		add_destroy_callback(free, bootblock);
		first_run = 0;

		/* read SPD from CBFS entry located within bootblock region */
		if (flashrom_read(bootblock, size,
				  INTERNAL_BUS_SPI, "BOOT_STUB") < 0)
			return -1;
	}

	if ((file = cbfs_find("spd.bin", bootblock, size)) == NULL)
		return -1;

	spd_index = rambi_get_spd_index(intf);
	if (spd_index < 0)
		return -1;

	spd_offset = ntohl(file->offset) + (spd_index * 256);
	lprintf(LOG_DEBUG, "Using memory config %u\n", spd_index);
	memcpy(buf, (void *)file + spd_offset + reg, len);

	return len;
}

static int rambi_spd_read(struct platform_intf *intf,
			 int dimm, int reg, int len, uint8_t *buf)
{
	return rambi_spd_read_cbfs(intf, dimm, reg, len, buf);
}

static struct memory_spd_cb rambi_spd_cb = {
	.read		= rambi_spd_read,
};

struct memory_cb rambi_memory_cb = {
	.dimm_count	= rambi_dimm_count,
	.dimm_speed	= rambi_dimm_speed,
	.spd		= &rambi_spd_cb,
};
