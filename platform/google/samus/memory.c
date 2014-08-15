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
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"

#include "lib/cbfs_core.h"
#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/math.h"
#include "lib/spd.h"

#include "samus.h"

#define SAMUS_DIMM_COUNT	2

/*
 * SPD blob contains up to eight entries which are selected by
 * board strappings.
 *
 * GPIO67: Bit 0
 * GPIO68: Bit 1
 * GPIO69: Bit 2
 * GPIO65: Bit 3
 */
static int samus_get_spd_index(struct platform_intf *intf)
{
	int spd_index = 0;
	int val;
	struct gpio_map ram_id0 = { 67, GPIO_IN, 0, 2, 3 };
	struct gpio_map ram_id1 = { 68, GPIO_IN, 0, 2, 4 };
	struct gpio_map ram_id2 = { 69, GPIO_IN, 0, 2, 5 };
	struct gpio_map ram_id3 = { 65, GPIO_IN, 0, 2, 2 };

	if ((val = intf->cb->gpio->read(intf, &ram_id0)) < 0)
		return -1;
	spd_index |= val;

	if ((val = intf->cb->gpio->read(intf, &ram_id1)) < 0)
		return -1;
	spd_index |= val << 1;

	if ((val = intf->cb->gpio->read(intf, &ram_id2)) < 0)
		return -1;
	spd_index |= val << 2;

	if ((val = intf->cb->gpio->read(intf, &ram_id3)) < 0)
		return -1;
	spd_index |= val << 3;

	return spd_index;
}

/*
 * samus_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int samus_dimm_count(struct platform_intf *intf)
{
	return SAMUS_DIMM_COUNT;
}

static int samus_spd_read_cbfs(struct platform_intf *intf,
				int dimm, int reg, int len, uint8_t *buf)
{
	static int first_run = 1;
	static uint8_t *bootblock = NULL;
	size_t size = SAMUS_HOST_FIRMWARE_ROM_SIZE;
	struct cbfs_file *file;
	int spd_index = 0;
	uint32_t spd_offset;

	if (dimm > samus_dimm_count(intf)) {
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

	spd_index = samus_get_spd_index(intf);
	if (spd_index < 0)
		return -1;

	spd_offset = ntohl(file->offset) + (spd_index * 256);
	lprintf(LOG_DEBUG, "Using memory config %u\n", spd_index);
	memcpy(buf, (void *)file + spd_offset + reg, len);

	return len;
}

static int samus_spd_read(struct platform_intf *intf,
			 int dimm, int reg, int len, uint8_t *buf)
{
	return samus_spd_read_cbfs(intf, dimm, reg, len, buf);
}

static struct memory_spd_cb samus_spd_cb = {
	.read		= samus_spd_read,
};

struct memory_cb samus_memory_cb = {
	.dimm_count	= samus_dimm_count,
	.spd		= &samus_spd_cb,
};
