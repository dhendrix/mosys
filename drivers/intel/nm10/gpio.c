/*
 * Copyright (C) 2010 Google Inc.
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
 * gpio.c: GPIO functions for NM10
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "intf/io.h"
#include "intf/pci.h"

#include "drivers/gpio.h"
#include "drivers/intel/nm10.h"

static uint16_t nm10_get_gpio_base(struct platform_intf *intf)
{
	static uint16_t nm10_gpio_base = 0;

	if (nm10_gpio_base != 0)
		return nm10_gpio_base;

	if (pci_read16(intf, 0, 31, 0, 0x48, &nm10_gpio_base) < 0) {
		lprintf(LOG_DEBUG, "NM10 GPIO: Unable to find base\n");
		return 0;
	}

	/* lsb is enable bit */
	nm10_gpio_base &= 0xfffe;

	lprintf(LOG_DEBUG, "NM10 GPIO: base is 0x%04x\n", nm10_gpio_base);

	return nm10_gpio_base;
}

int nm10_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	uint16_t gpio_base = 0;
	uint16_t port_offset[] = { 0x0c, 0x38 };	/* GP_LVL and GP_LVL2 */
	uint32_t data;

	if (gpio->port > sizeof(port_offset)) {
		lprintf(LOG_DEBUG, "NM10 GPIO: Invalid port %d\n",
			gpio->port);
		return -1;
	}

	gpio_base = nm10_get_gpio_base(intf);
	if (!gpio_base) {
		lprintf(LOG_DEBUG, "NM10 GPIO: Unable to find base\n");
		return -1;
	}

	/* read gpio level */
	io_read32(intf, gpio_base + port_offset[gpio->port], &data);
	return ((data >> gpio->pin) & 1);
}

int nm10_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state)
{
	uint16_t gpio_base = 0;
	uint16_t port_offset[] = { 0x0c, 0x38 };
	uint32_t data;
	uint16_t addr;

	if (gpio->type != GPIO_OUT)
		return -1;

	if (gpio->port > sizeof(port_offset)) {
		lprintf(LOG_DEBUG, "NM10 GPIO: Invalid port %d\n",
			gpio->port);
		return -1;
	}

	gpio_base = nm10_get_gpio_base(intf);
	if (!gpio_base) {
		lprintf(LOG_DEBUG, "NM10 GPIO: Unable to find base\n");
		return -1;
	}

	/* read current level */
	addr = gpio_base + port_offset[gpio->port];
	io_read32(intf, addr, &data);

	switch (state) {
	case 0:
		if (!(data & (1 << gpio->pin))) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 0\n", gpio->name);
			return 0;
		}
		lprintf(LOG_DEBUG, "NM10 GPIO: mask 0x%08x -> 0x%08x\n",
			data, data & ~(1 << gpio->pin));

		io_write32(intf, addr, data & ~(1 << gpio->pin));
		break;
	case 1:
		if (data & (1 << gpio->pin)) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 1\n", gpio->name);
			return 0;
		}
		lprintf(LOG_DEBUG, "NM10 GPIO: mask 0x%08x -> 0x%08x\n",
			data, data | (1 << gpio->pin));

		io_write32(intf, addr, data | (1 << gpio->pin));
		break;
	default:
		lprintf(LOG_ERR, "Invaild state %d\n", state);
		return -1;
	}

	return 0;
}

/*
 * nm10_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int nm10_gpio_list(struct platform_intf *intf)
{
	int i, state;
	uint32_t gpiobase;
	uint32_t gpio_use_sel = 0, gpio_use_sel2 = 0;
	uint32_t gpio_io_sel = 0, gpio_io_sel2 = 0;
	uint32_t gpio_lvl = 0, gpio_lvl2 = 0;
//	uint32_t gpio_inv = 0;
	/* many GPIOs in the second bank are not implemented */
	int bank2_gpios[] = { 0, 1, 2, 3, 4, 5, 6, 7, 16, 17 };

	gpiobase = nm10_get_gpio_base(intf);

	io_read32(intf, gpiobase + 0x00, &gpio_use_sel);
	io_read32(intf, gpiobase + 0x04, &gpio_io_sel);
	io_read32(intf, gpiobase + 0x0c, &gpio_lvl);

	/* FIXME: do we care about this? it does not actually effect gpio_lvl */
//	io_read32(intf, gpiobase + 0x2c, &gpio_inv);

	io_read32(intf, gpiobase + 0x30, &gpio_use_sel2);
	io_read32(intf, gpiobase + 0x34, &gpio_io_sel2);
	io_read32(intf, gpiobase + 0x38, &gpio_lvl2);

	for (i = 0; i < 32; i++) {
		struct gpio_map gpio;
		char tmp[16];

		/* skip if signal is "native function" instead of GPIO */
		if ((gpio_use_sel & (1 << i)) == 0)
			continue;

		gpio.id = i;

		if ((gpio_io_sel & (1 << i)) == 0)
			gpio.type = GPIO_OUT;
		else
			gpio.type = GPIO_IN;

		gpio.dev = 0;
		gpio.port = 0;		/* port in device */
		gpio.pin = i;		/* pin in port in device */
		gpio.neg = 0;

		sprintf(tmp, "NM10");
		gpio.devname = mosys_strdup(tmp);

		sprintf(tmp, "GPIO%02d", i);
		gpio.name = mosys_strdup(tmp);

		state = nm10_read_gpio(intf, &gpio);

		if (state < 0)
			continue;

		kv_pair_print_gpio(&gpio, state);

		free((void *)gpio.devname);
		free((void *)gpio.name);
	}

	for (i = 0; i < sizeof(bank2_gpios) / sizeof(bank2_gpios[0]); i++) {
		struct gpio_map gpio;
		char tmp[16];

		/* skip if signal is "native function" instead of GPIO */
		if ((gpio_use_sel2 & (1 << bank2_gpios[i])) == 0)
			continue;

		gpio.id = bank2_gpios[i];

		if ((gpio_io_sel2 & (1 << bank2_gpios[i])) == 0)
			gpio.type = GPIO_OUT;
		else
			gpio.type = GPIO_IN;

		gpio.dev = 0;
		gpio.port = 1;			/* port in device */
		gpio.pin = bank2_gpios[i];	/* pin in port in device */

		sprintf(tmp, "NM10");
		gpio.devname = mosys_strdup(tmp);

		sprintf(tmp, "GPIO%02d", bank2_gpios[i] + 32);
		gpio.name = mosys_strdup(tmp);

		state = nm10_read_gpio(intf, &gpio);

		if (state < 0)
			continue;

		kv_pair_print_gpio(&gpio, state);

		free((void *)gpio.devname);
		free((void *)gpio.name);
	}

	return 0;
}
