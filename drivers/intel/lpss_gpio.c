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
 *
 * gpio.c: GPIO functions for Intel LPSS chipsets
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "intf/io.h"
#include "intf/pci.h"

#include "drivers/gpio.h"
#include "drivers/intel/ich_generic.h"
#include "drivers/intel/lpss_generic.h"

int lpss_get_gpio_base(struct platform_intf *intf, uint32_t *val)
{
	static uint32_t lpss_gpio_base;

	if (lpss_gpio_base) {
		*val = lpss_gpio_base;
		return 0;
	}

	if (pci_read32(intf, 0, 31, 0, 0x48, &lpss_gpio_base) < 0) {
		lprintf(LOG_DEBUG, "LPSS GPIO: unable to find base\n");
		return -1;
	}

	/* lsb is enable bit */
	lpss_gpio_base &= ~1;

	lprintf(LOG_DEBUG, "LPSS GPIO: base is 0x%08x\n", lpss_gpio_base);
	*val = lpss_gpio_base;
	return 0;
}

/* returns 1 if GPIO is valid, 0 otherwise */
static int lpss_gpio_valid(enum ich_generation gen, struct gpio_map *gpio)
{
	switch (gen) {
	case ICH_8_LPSS:
		/* Series 8 LPSS supports 94 GPIOs */
		if (gpio->id > 94) {
			lprintf(LOG_ERR, "LPSS GPIO: invalid ID %d\n",
				gpio->id);
			return 0;
		}
		break;
	default:
		lprintf(LOG_ERR, "LPSS GPIO: invalid ICH gen %d\n", gen);
		return 0;
	}

	return 1;
}

int lpss_read_gpio(struct platform_intf *intf,
		   enum ich_generation gen, struct gpio_map *gpio)
{
	uint32_t base, val;

	if (!lpss_gpio_valid(gen, gpio))
		return -1;
	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;
	if (io_read32(intf, base + LPSS_GPIO_CONF0(gpio->id), &val) < 0)
		return -1;

	return ((val >> LPSS_GPIO_CONF0_GPI_BIT) & 1);
}

int lpss_set_gpio(struct platform_intf *intf, enum ich_generation gen,
		  struct gpio_map *gpio, int state)
{
	uint32_t base, addr, val;

	if (gpio->type != GPIO_OUT) {
		lprintf(LOG_DEBUG, "Must set \"%s\" to output mode to change "
		        "its value\n", gpio->name ? gpio->name : "Unknown");
		return -1;
	}

	if (!lpss_gpio_valid(gen, gpio))
		return -1;
	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;

	addr = base + LPSS_GPIO_CONF0(gpio->id);
	if (io_read32(intf, addr, &val) < 0)
		return -1;

	/* Check if the GPIO is native mode */
	if (!(val & (1 << LPSS_GPIO_CONF0_MODE_BIT))) {
		lprintf(LOG_DEBUG, "Changing GPIO ID %d from native to "
			"GPIO mode\n", gpio->id);

		val |= (1 << LPSS_GPIO_CONF0_MODE_BIT);
		if (io_write32(intf, addr, val) < 0)
			return -1;
	}

	/* Check if the GPIO is output */
	if (val & (1 << LPSS_GPIO_CONF0_DIR_BIT)) {
		lprintf(LOG_DEBUG, "Changing GPIO ID %d from input to "
		        "output\n", gpio->id);

		val &= ~(1 << LPSS_GPIO_CONF0_DIR_BIT);
		if (io_write32(intf, addr, val) < 0)
			return -1;
	}

	/* Set the output value */
	switch (state) {
	case 0:
		if (!(val & (1 << LPSS_GPIO_CONF0_GPO_BIT))) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 0\n", gpio->name);
		} else {
			val &= ~LPSS_GPIO_CONF0_GPO_BIT;
			if (io_write32(intf, addr, val) < 0)
				return -1;
		}
		break;
	case 1:
		if (val & (1 << LPSS_GPIO_CONF0_GPO_BIT)) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 1\n", gpio->name);
		} else {
			val |= (1 << LPSS_GPIO_CONF0_GPO_BIT);
			if (io_write32(intf, addr, val) < 0)
				return -1;
		}
		break;
	default:
		lprintf(LOG_ERR, "Invaild state %d\n", state);
		return -1;
	}

	return 0;
}

int lpss_gpio_list(struct platform_intf *intf, enum ich_generation gen,
		   int gpio_ids[], int num_gpios)
{
	int i, state;
	uint32_t base, addr, val;

	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;

	for (i = 0; i < num_gpios; i++) {
		struct gpio_map gpio;
		char tmp[16];

		memset(&gpio, 0, sizeof(gpio));

		gpio.id = gpio_ids[i];
		gpio.port = gpio.id % 32;
		gpio.pin = gpio.id / 32;

		if (!lpss_gpio_valid(gen, &gpio))
			return -1;

		addr = base + LPSS_GPIO_CONF0(gpio.id);
		if (io_read32(intf, addr, &val) < 0)
			return -1;

		/* skip if signal is "native function" instead of GPIO */
		if (!(val & (1 << LPSS_GPIO_CONF0_MODE_BIT)))
			continue;

		if (val & (1 << LPSS_GPIO_CONF0_DIR_BIT))
			gpio.type = GPIO_IN;
		else
			gpio.type = GPIO_OUT;

		if (val & (1 << LPSS_GPIO_CONF0_INV_BIT))
			gpio.neg = 1;

		sprintf(tmp, "LPSS");
		gpio.devname = mosys_strdup(tmp);

		sprintf(tmp, "GPIO%02d", gpio.id);
		gpio.name = mosys_strdup(tmp);

		state = lpss_read_gpio(intf, gen, &gpio);
		if (state >= 0)
			kv_pair_print_gpio(&gpio, state);

		free((void *)gpio.devname);
		free((void *)gpio.name);
	}

	return 0;
}
