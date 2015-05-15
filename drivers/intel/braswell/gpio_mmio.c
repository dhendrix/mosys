/*
 * Copyright 2015, Google Inc.
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
 * gpio_mmio.c: GPIO functions for Braswell. This driver uses only the MMIO
 * (non-legacy) GPIO interface.
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "intf/mmio.h"
#include "intf/pci.h"

#include "drivers/gpio.h"
#include "drivers/intel/braswell.h"

/* GPIO bank descriptions. */
/* North community */
static const struct braswell_gpio_bank braswell_gpncore_bank = {
	.gpio_count			= BSW_GPNCORE_COUNT,
	.community_base_offset		= BSW_GPNCORE_OFFSET,
};

/* South east community */
static const struct braswell_gpio_bank braswell_gpsecore_bank = {
	.gpio_count			= BSW_GPSECORE_COUNT,
	.community_base_offset		= BSW_GPSECORE_OFFSET,
};

/* South west community */
static const struct braswell_gpio_bank braswell_gpswcore_bank = {
	.gpio_count			= BSW_GPSWCORE_COUNT,
	.community_base_offset		= BSW_GPSWCORE_OFFSET,
};

/* East community */
static const struct braswell_gpio_bank braswell_gpecore_bank = {
	.gpio_count			= BSW_GPECORE_COUNT,
	.community_base_offset		= BSW_GPECORE_OFFSET,
};

/* Get the GPIO IO base address. Returns 0 on success, -1 otherwise. */
static int braswell_get_io_base(struct platform_intf *intf, uint32_t *val)
{
	static uint32_t braswell_io_base;

	if (braswell_io_base) {
		*val = braswell_io_base;
		return 0;
	}

	if (pci_read32(intf, 0, 31, 0, 0x4C, &braswell_io_base) < 0) {
		lprintf(LOG_ERR, "BRASWELL GPIO: unable to find base\n");
		return -1;
	}

	braswell_io_base &= 0xffff0000;

	*val = braswell_io_base;
	return 0;
}

/* Maps port to bank structure. Returns 0 on success, -1 otherwise. */
static int braswell_gpio_port_to_bank(enum braswell_gpio_port port,
				      const struct braswell_gpio_bank **bank)
{
	int ret = 0;

	switch (port) {
	case BSW_GPNCORE_PORT:
		*bank = &braswell_gpncore_bank;
		break;
	case BSW_GPSECORE_PORT:
		*bank = &braswell_gpsecore_bank;
		break;
	case BSW_GPSWCORE_PORT:
		*bank = &braswell_gpswcore_bank;
		break;
	case BSW_GPECORE_PORT:
		*bank = &braswell_gpecore_bank;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}


/*
 * braswell_read_gpio   - read GPIO status
 * @intf:		platform interface
 * @gpio:		gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */

int braswell_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	uint32_t io_base, val, dir;
	const struct braswell_gpio_bank *bank;

	if (braswell_get_io_base(intf, &io_base) < 0)
		return -1;
	if (braswell_gpio_port_to_bank(gpio->port, &bank) < 0)
		return -1;
	if (mmio_read32(intf, BSW_GPIO_CONF0(io_base, bank, gpio),
			&val) < 0)
		return -1;

	/* Gpio direction can be read from the PAD_CFG0 register */
	dir = (val & BSW_GPIO_CFG_MASK) >> 8;
	switch (dir) {
	case BSW_GPIO_IN_OUT: /* IN and OUT */
	case BSW_GPIO_IN: /* IN */
		gpio->type = GPIO_IN;
		break;
	case BSW_GPIO_OUT: /* OUT */
		gpio->type = GPIO_OUT;
		break;
	case BSW_GPIO_HIZ: /* Hi-Z */
		gpio->type = GPIO_ALT;
		break;
	}

	return (val & BSW_GPIO_RX_STAT);
}

/*
 * braswell_set_gpio    - set GPIO status to state
 * @intf:       platform interface
 * @gpio:       gpio map
 * @state:     0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int braswell_set_gpio(struct platform_intf *intf, struct gpio_map *gpio,
		      int state)
{
	uint32_t io_base, val;
	const struct braswell_gpio_bank *bank;

	if (gpio->type != GPIO_OUT) {
		lprintf(LOG_ERR, "Must set \"%s\" to out mode to change its val\n"
		, gpio->name ? gpio->name : "Unknown");
		return -1;
	}

	if (braswell_get_io_base(intf, &io_base) < 0)
		return -1;
	if (braswell_gpio_port_to_bank(gpio->port, &bank) < 0)
		return -1;
	if (mmio_read32(intf, BSW_GPIO_CONF0(io_base, bank, gpio),
			&val) < 0)
		return -1;

	/* Set the output value */
	if (state)
		val = val | BSW_GPIO_TX_STAT;
	else
		val = val & (~BSW_GPIO_TX_STAT);

	mmio_write32(intf, BSW_GPIO_CONF0(io_base, bank, gpio), val);

	return 0;
}
