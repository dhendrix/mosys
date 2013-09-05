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

#include <limits.h>	/* for INT_MAX */

#include <arpa/inet.h>	/* ntohl() */

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/series6.h"

#include "lib/cbfs_core.h"
#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/math.h"
#include "lib/spd.h"

#include "link.h"

#define LINK_DIMM_COUNT	2

/*
 * link_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int link_dimm_count(struct platform_intf *intf)
{
	return LINK_DIMM_COUNT;
}

static int link_spd_read_cbfs(struct platform_intf *intf,
			      int dimm, int reg, int len, uint8_t *buf)
{
	int rc = -1;
	static int first_run = 1;
	static uint8_t *bootblock = NULL;
	size_t size = LINK_HOST_FIRMWARE_ROM_SIZE;
	struct cbfs_file *file;
	uint8_t spd_index = 0;
	uint32_t spd_offset;

	if (dimm > link_dimm_count(intf)) {
		lprintf(LOG_DEBUG, "%s: Invalid DIMM specified\n", __func__);
		goto link_spd_read_cbfs_exit;
	}

	if (first_run) {
		bootblock = mosys_malloc(size);	/* FIXME: overkill */
		add_destroy_callback(free, bootblock);
		first_run = 0;

		/* read SPD from CBFS entry located within bootblock region */
		if (flashrom_read(bootblock, size,
				  INTERNAL_BUS_SPI, "BOOT_STUB") < 0)
			goto link_spd_read_cbfs_exit;
	}

	if ((file = cbfs_find("spd.bin", bootblock, size)) == NULL)
		goto link_spd_read_cbfs_exit;

	/*
	 * SPD blob contains up to six entries which are selected by
	 * board strappings.
	 *
	 * GPIO41: Bit 0
	 * GPIO42: Bit 1
	 * GPIO43: Bit 2
	 * GPIO10: Bit 3
	 */
	{
		int val;
		struct gpio_map gpio41 = { 41, GPIO_IN, LINK_GPIO_PCH, 1,  9 };
		struct gpio_map gpio42 = { 42, GPIO_IN, LINK_GPIO_PCH, 1, 10 };
		struct gpio_map gpio43 = { 43, GPIO_IN, LINK_GPIO_PCH, 1, 11 };
		struct gpio_map gpio10 = { 10, GPIO_IN, LINK_GPIO_PCH, 0, 10 };

		if ((val = intf->cb->gpio->read(intf, &gpio41)) < 0)
			goto link_spd_read_cbfs_exit;
		spd_index |= val;

		if ((val = intf->cb->gpio->read(intf, &gpio42)) < 0)
			goto link_spd_read_cbfs_exit;
		spd_index |= val << 1;

		if ((val = intf->cb->gpio->read(intf, &gpio43)) < 0)
			goto link_spd_read_cbfs_exit;
		spd_index |= val << 2;

		if ((val = intf->cb->gpio->read(intf, &gpio10)) < 0)
			goto link_spd_read_cbfs_exit;
		spd_index |= val << 3;
	}

	spd_offset = ntohl(file->offset) + (spd_index * 256);
	lprintf(LOG_DEBUG, "Using memory config %u\n", spd_index);
	memcpy(buf, (void *)file + spd_offset + reg, len);
	rc = len;
link_spd_read_cbfs_exit:
	return rc;
}

static int link_spd_read(struct platform_intf *intf,
			 int dimm, int reg, int len, uint8_t *buf)
{
	return link_spd_read_cbfs(intf, dimm, reg, len, buf);
}

static struct memory_spd_cb link_spd_cb = {
	.read		= link_spd_read,
};

struct memory_cb link_memory_cb = {
	.dimm_count	= link_dimm_count,
	.spd		= &link_spd_cb,
};
