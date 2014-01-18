/*
 * Copyright 2014, Google Inc.
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

/*
 * gpio_mmio.c: GPIO functions for Baytrail. This driver uses only the MMIO
 * (non-legacy) GPIO interface.
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "intf/mmio.h"
#include "intf/pci.h"

#include "drivers/gpio.h"
#include "drivers/intel/baytrail.h"

/* These LUTs map GPIO# to pad index. In MMIO mode, GPIOs are addressed
 * according to this pad index. */
static const uint8_t gpncore_gpio_to_pad[BAYTRAIL_GPNCORE_COUNT] =
	{ 19, 18, 17, 20, 21, 22, 24, 25,	/* [ 0: 7] */
	  23, 16, 14, 15, 12, 26, 27,  1,	/* [ 8:15] */
	   4,  8, 11,  0,  3,  6, 10, 13,	/* [16:23] */
	   2,  5,  9 };				/* [24:26] */

static const uint8_t gpscore_gpio_to_pad[BAYTRAIL_GPSCORE_COUNT] =
	{  85,  89, 93,  96, 99, 102,  98, 101,	/* [ 0:  7] */
	   34,  37, 36,  38, 39,  35,  40,  84,	/* [ 8: 15] */
	   62,  61, 64,  59, 54,  56,  60,  55,	/* [16: 23] */
	   63,  57, 51,  50, 53,  47,  52,  49,	/* [24: 31] */
	   48,  43, 46,  41, 45,  42,  58,  44,	/* [32: 39] */
	   95, 105, 70,  68, 67,  66,  69,  71,	/* [40: 47] */
	   65,  72, 86,  90, 88,  92, 103,  77,	/* [48: 55] */
	   79,  83, 78,  81, 80,  82,  13,  12,	/* [56: 63] */
	   15,  14, 17,  18, 19,  16,   2,   1,	/* [64: 71] */
	    0,   4,  6,   7,  9,   8,  33,  32,	/* [72: 79] */
	   31,  30, 29,  27, 25,  28,  26,  23,	/* [80: 87] */
	   21,  20, 24,  22,  5,   3,  10,  11,	/* [88: 95] */
	  106,  87, 91, 104, 97, 100 };		/* [96:101] */

static const uint8_t gpssus_gpio_to_pad[BAYTRAIL_GPSSUS_COUNT] =
	{ 29, 33, 30, 31, 32, 34, 36, 35,	/* [ 0: 7] */
	  38, 37, 18,  7, 11, 20, 17,  1,	/* [ 8:15] */
	   8, 10, 19, 12,  0,  2, 23, 39,	/* [16:23] */
	  28, 27, 22, 21, 24, 25, 26, 51,	/* [24:31] */
	  56, 54, 49, 55, 48, 57, 50, 58,	/* [32:39] */
	  52, 53, 59, 40 };			/* [40:43] */

/* GPIO bank descriptions. */
static const struct baytrail_gpio_bank baytrail_gpncore_bank = {
	.gpio_count 		= BAYTRAIL_GPNCORE_COUNT,
	.gpio_to_pad 		= gpncore_gpio_to_pad,
	.io_base_offset		= BAYTRAIL_GPNCORE_OFFSET,
};

static const struct baytrail_gpio_bank baytrail_gpscore_bank = {
	.gpio_count 		= BAYTRAIL_GPSCORE_COUNT,
	.gpio_to_pad 		= gpscore_gpio_to_pad,
	.io_base_offset		= BAYTRAIL_GPSCORE_OFFSET,
};

static const struct baytrail_gpio_bank baytrail_gpssus_bank = {
	.gpio_count 		= BAYTRAIL_GPSSUS_COUNT,
	.gpio_to_pad 		= gpssus_gpio_to_pad,
	.io_base_offset		= BAYTRAIL_GPSSUS_OFFSET,
};

/* Assigns the IO base address to *val. Returns 0 on success, -1 otherwise. */
static int baytrail_get_io_base(struct platform_intf *intf, uint32_t *val)
{
	static uint32_t baytrail_io_base;

	if (baytrail_io_base) {
		*val = baytrail_io_base;
		return 0;
	}

	if (pci_read32(intf, 0, 31, 0, 0x4c, &baytrail_io_base) < 0) {
		lprintf(LOG_DEBUG, "BAYTRAIL GPIO: unable to find base\n");
		return -1;
	}

	/* Bits 31:14 contain the base address. */
	baytrail_io_base &= 0xffffc000;

	lprintf(LOG_DEBUG, "BAYTRAIL GPIO: base is 0x%08x\n",
		baytrail_io_base);
	*val = baytrail_io_base;
	return 0;
}

/* Maps port to bank structure. Returns 0 on success, -1 otherwise. */
static int baytrail_gpio_port_to_bank(enum baytrail_gpio_port port,
				      const struct baytrail_gpio_bank **bank)
{
	int ret = 0;

	switch (port) {
	case BAYTRAIL_GPNCORE_PORT:
		*bank = &baytrail_gpncore_bank;
		break;
	case BAYTRAIL_GPSCORE_PORT:
		*bank = &baytrail_gpscore_bank;
		break;
	case BAYTRAIL_GPSSUS_PORT:
		*bank = &baytrail_gpssus_bank;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

/* Returns 1 if the GPIO is valid, 0 otherwise. */
static int baytrail_gpio_is_valid(struct gpio_map *gpio)
{
	const struct baytrail_gpio_bank *bank;

	if (baytrail_gpio_port_to_bank(gpio->port, &bank) != 0) {
		lprintf(LOG_ERR, "BAYTRAIL GPIO: invalid port %d\n",
			gpio->port);
		return 0;
	}

	if (gpio->id > bank->gpio_count) {
		lprintf(LOG_ERR, "BAYTRAIL_GPIO: invalid gpio %d\n",
			gpio->id);
		return 0;
	}

	return 1;
}

/*
 * baytrail_read_gpio   - read GPIO status
 * @intf:		platform interface
 * @gpio:		gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
int baytrail_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	uint32_t base, val;
	const struct baytrail_gpio_bank *bank;

	if (!baytrail_gpio_is_valid(gpio))
		return -1;
	if (baytrail_get_io_base(intf, &base) < 0)
		return -1;
	if (baytrail_gpio_port_to_bank(gpio->port, &bank) < 0)
		return -1;
	if (mmio_read32(intf, BAYTRAIL_GPIO_VAL(base, bank, gpio->id),
			&val) < 0)
		return -1;

	return ((val >> BAYTRAIL_GPIO_VAL_GPIO_BIT) & 1);
}

/*
 * baytrail_set_gpio    - set GPIO status
 * @intf:       platform interface
 * @gpio:       gpio map
 * @status:     0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int baytrail_set_gpio(struct platform_intf *intf, struct gpio_map *gpio,
		      int state)
{
	uint32_t base, val;
	const struct baytrail_gpio_bank *bank;

	if (gpio->type != GPIO_OUT) {
		lprintf(LOG_DEBUG, "Must set \"%s\" to output mode to change "
			"its value\n", gpio->name ? gpio->name : "Unknown");
		return -1;
	}

	if (!baytrail_gpio_is_valid(gpio))
		return -1;
	if (baytrail_get_io_base(intf, &base) < 0)
		return -1;
	if (baytrail_gpio_port_to_bank(gpio->port, &bank) < 0)
		return -1;
	if (mmio_read32(intf, BAYTRAIL_GPIO_VAL(base, bank, gpio->id),
			&val) < 0)
		return -1;

	if ((val >> BAYTRAIL_GPIO_VAL_INPUT_DISABLE_BIT) & 1) {
		lprintf(LOG_DEBUG, "BAYTRAIL_GPIO: Output not enabled for "
			"GPIO %d\n", gpio->id);
		return -1;
	}

	/* Set the output value */
	val = (val & (~0x01)) | (state & 0x01);
	mmio_write32(intf, BAYTRAIL_GPIO_VAL(base, bank, gpio->id), val);

	return 0;
}
