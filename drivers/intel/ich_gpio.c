/*
 * Copyright (C) 2011 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
	uint32_t gpio_base;
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
		lprintf(LOG_DEBUG, "ICH GPIO: pin %d of port %d is not"
			"an output\n", gpio->pin, gpio->port);
		return -1;
	}

	if (ich_get_regs(intf, gen, gpio) < 0)
		return -1;

	addr = ich_gpio_offsets.gpio_base + ich_gpio_offsets.lvl;
	if (io_read32(intf, addr, &val) < 0)
		return -1;

	switch (state) {
	case 0:
		if (!(val & (1 << gpio->pin))) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 0\n", gpio->name);
			return 0;
		}
		lprintf(LOG_DEBUG, "ICH GPIO: mask 0x%08x -> 0x%08x\n",
			val, val & ~(1 << gpio->pin));

		if (io_write32(intf, addr, val & ~(1 << gpio->pin)) < 0)
			return -1;
		break;
	case 1:
		if (val & (1 << gpio->pin)) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 1\n", gpio->name);
			return 0;
		}
		lprintf(LOG_DEBUG, "ICH GPIO: mask 0x%08x -> 0x%08x\n",
			val, val | (1 << gpio->pin));

		if (io_write32(intf, addr, val | (1 << gpio->pin)) < 0)
			return -1;
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
