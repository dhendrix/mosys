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
#include "drivers/google/cros_ec.h"
#include "drivers/intel/baytrail.h"

#include "lib/cbfs_core.h"
#include "lib/file.h"
#include "lib/flashrom.h"
#include "lib/math.h"
#include "lib/spd.h"

#include "strago.h"

#define STRAGO_DIMM_COUNT	2
#define STRAGO_DIMM_SPEED	1333

/*
 * strago_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int strago_dimm_count(struct platform_intf *intf)
{
	/*
	 * TODO(shawnn): Find a programatic way to do this detection.
	 * Platforms have 1 or 2 DIMM config based on RAM_ID.
	 */
	return STRAGO_DIMM_COUNT;
}

/*
 * strago_dimm_speed - Write actual DDR speed in MHz to kv
 *
 * @intf:	platform interface
 * @dimm:	DIMM number
 * @kv:		kv_pair structure
 *
 * returns actual DDR speed in MHz
 */
static int strago_dimm_speed(struct platform_intf *intf,
			    int dimm,
			    struct kv_pair *kv) {
	int speed = STRAGO_DIMM_SPEED;
	if (kv)
		kv_pair_fmt(kv, "speed", "%d MHz", speed);
	return speed;
}

static int strago_spd_read_cbfs(struct platform_intf *intf,
				int dimm, int reg, int len, uint8_t *buf)
{
	static int first_run = 1;
	static uint8_t *bootblock = NULL;
	size_t size = STRAGO_HOST_FIRMWARE_ROM_SIZE;
	struct cbfs_file *file;
	int spd_index = 0;
	uint32_t spd_offset;

	if (dimm > strago_dimm_count(intf)) {
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

	/* Use an spd index of 0, as spd.bin for strago has only 1 entry */
	spd_offset = ntohl(file->offset) + (spd_index * 256);
	lprintf(LOG_DEBUG, "Using memory config %u\n", spd_index);
	memcpy(buf, (void *)file + spd_offset + reg, len);

	return len;
}

static int strago_spd_read(struct platform_intf *intf,
			 int dimm, int reg, int len, uint8_t *buf)
{
	return strago_spd_read_cbfs(intf, dimm, reg, len, buf);
}

static struct memory_spd_cb strago_spd_cb = {
	.read		= strago_spd_read,
};

struct memory_cb strago_memory_cb = {
	.dimm_count	= strago_dimm_count,
	.dimm_speed	= strago_dimm_speed,
	.spd		= &strago_spd_cb,
};