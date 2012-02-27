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
 *
 * gpio.c: GPIO functions for ICH chipsets
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "intf/io.h"
#include "intf/pci.h"

#include "drivers/gpio.h"
#include "drivers/intel/nm10.h"

/* For ports 0, 1, and 2 */
static uint8_t ich_use_sel_offsets[] = { 0x00, 0x30, 0x40 };
static uint8_t ich_io_sel_offsets[] = { 0x04, 0x34, 0x44 };
static uint8_t ich_lvl_offsets[] = { 0x0c, 0x38, 0x48 };
static uint8_t ich_gpio_inv_offset = 0x2c;	/* for port 1 only */

struct {
	uint16_t gpio_base;
	uint8_t use_sel;
	uint8_t io_sel;
	uint8_t lvl;
} ich_gpio_offsets;

/* returns 1 if GPIO is valid, 0 otherwise */
static int ich_gpio_valid(enum ich_generation gen, struct gpio_map *gpio)
{
	/* GP_LVL registers represent 32 pins each */
	if (gpio->pin > 31) {
		lprintf(LOG_DEBUG, "ICH GPIO: invalid pin %d\n", gpio->pin);
		return 0;
	}

	switch(gen) {
	case ICH7:
	case ICH8:
	case ICH9:
	case ICH10:		/* ICH10 corporate vs. consumer sku */
		if (gpio->port > 1) {
			lprintf(LOG_DEBUG, "ICH GPIO: invalid port %d\n",
			        gpio->port);
			return 0;
		}
		break;
	case ICH_6_SERIES:
	case ICH_7_SERIES:
		if (gpio->port > 2) {
			lprintf(LOG_DEBUG, "ICH GPIO: invalid port %d\n",
			        gpio->port);
			return 0;
		}
		break;
	default:
		lprintf(LOG_DEBUG, "ICH GPIO: invalid port %d\n",
		        gpio->port);
		return 0;
	}

	return 1;
}

int ich_get_gpio_base(struct platform_intf *intf, uint16_t *val)
{
	static uint16_t ich_gpio_base = 0;

	if (ich_gpio_base) {
		*val = ich_gpio_base;
		return 0;
	}

	if (pci_read16(intf, 0, 31, 0, 0x48, &ich_gpio_base) < 0) {
		lprintf(LOG_DEBUG, "ICH GPIO: unable to find base\n");
		return -1;
	}

	/* lsb is enable bit */
	ich_gpio_base &= ~1;

	lprintf(LOG_DEBUG, "ICH GPIO: base is 0x%08x\n", ich_gpio_base);
	*val = ich_gpio_base;
	return 0;
}

/* select register set for given port */
static int ich_get_regs(struct platform_intf *intf,
                        enum ich_generation gen, struct gpio_map *gpio)
{
	if (!ich_gpio_valid(gen, gpio))
			return -1;

	if (ich_get_gpio_base(intf, &ich_gpio_offsets.gpio_base) < 0)
		return -1;

	ich_gpio_offsets.use_sel = ich_use_sel_offsets[gpio->port];
	ich_gpio_offsets.io_sel = ich_io_sel_offsets[gpio->port];
	ich_gpio_offsets.lvl = ich_lvl_offsets[gpio->port];

	return 0;
}

int ich_read_gpio(struct platform_intf *intf,
                  enum ich_generation gen, struct gpio_map *gpio)
{
	uint32_t addr, val;

	if (ich_get_regs(intf, gen, gpio) < 0)
		return -1;

	addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.lvl;
	if (io_read32(intf, addr, &val) < 0)
		return -1;

	return ((val >> gpio->pin) & 1);
}

int ich_set_gpio(struct platform_intf *intf, enum ich_generation gen,
                 struct gpio_map *gpio, int state)
{
	uint32_t addr, val;

	if (gpio->type != GPIO_OUT) {
		lprintf(LOG_DEBUG, "Must set \"%s\" to output mode to change "
		        "its value\n", gpio->name ? gpio->name : "Unknown");
		return -1;
	}

	if (ich_get_regs(intf, gen, gpio) < 0)
		return -1;

	addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.use_sel;
	if (io_read32(intf, addr, &val) < 0)
		return -1;
	if (!(val & (1 << gpio->pin))) {
		lprintf(LOG_DEBUG, "Changing port %d, pin %d from native to "
		        "GPIO mode\n", gpio->port, gpio->pin);
		val |= (1 << gpio->pin);
		if (io_write32(intf, addr, val) < 0)
			return -1;
	}

	addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.io_sel;
	if (io_read32(intf, addr, &val) < 0)
		return -1;
	if (val & (1 << gpio->pin)) {
		lprintf(LOG_DEBUG, "Changing port %d, pin %d from input to "
		        "output\n", gpio->port, gpio->pin);
		val &= ~(1 << gpio->pin);
		if (io_write32(intf, addr, val) < 0)
			return -1;
	}

	addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.lvl;
	if (io_read32(intf, addr, &val) < 0)
		return -1;
	switch (state) {
	case 0:
		if (!(val & (1 << gpio->pin))) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 0\n", gpio->name);
		} else {
			if (io_write32(intf, addr, val & ~(1 << gpio->pin)) < 0)
				return -1;
		}
		break;
	case 1:
		if (val & (1 << gpio->pin)) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 1\n", gpio->name);
		} else {
			if (io_write32(intf, addr, val | (1 << gpio->pin)) < 0)
				return -1;
		}
		break;
	default:
		lprintf(LOG_ERR, "Invaild state %d\n", state);
		return -1;
	}

	return 0;
}

int ich_gpio_list(struct platform_intf *intf, enum ich_generation gen,
                  int port, int gpio_pins[], int num_gpios)
{
	int i, state;
	uint32_t addr, val;

	for (i = 0; i < num_gpios; i++) {
		struct gpio_map gpio;
		char tmp[16];

		memset(&gpio, 0, sizeof(gpio));

		gpio.port = port;		/* port in device */
		gpio.pin = gpio_pins[i];	/* pin in port in device */
		gpio.id = gpio.port + gpio.pin;

		if (ich_get_regs(intf, gen, &gpio) < 0)
			return -1;

		/* skip if signal is "native function" instead of GPIO */
		addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.use_sel;
		if (io_read32(intf, addr, &val) < 0)
			return -1;
		if ((val & (1 << gpio.pin)) == 0)
			continue;

		addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.io_sel;
		if (io_read32(intf, addr, &val) < 0)
			return -1;
		if ((val & (1 << gpio.pin)) == 0)
			gpio.type = GPIO_OUT;
		else
			gpio.type = GPIO_IN;


		/* gpio negation only applies to pins 15:0 in port 1 (GP_LVL2
		   set) and only if the pin is configured as an input */
		if (port == 1 && gpio.pin < 16 && gpio.type == GPIO_IN) {
			addr = ich_gpio_offsets.gpio_base + ich_gpio_inv_offset;
			if (io_read32(intf, addr, &val) < 0)
				return -1;
			gpio.neg = (val >> gpio.pin) & 1;
		} else {
			gpio.neg = 0;
		}

		sprintf(tmp, "ICH");
		gpio.devname = mosys_strdup(tmp);

		sprintf(tmp, "GPIO%02d", gpio.id);
		gpio.name = mosys_strdup(tmp);

		state = ich_read_gpio(intf, gen, &gpio);
		if (state < 0)
			continue;

		kv_pair_print_gpio(&gpio, state);
		free((void *)gpio.devname);
		free((void *)gpio.name);
	}

	return 0;
}
