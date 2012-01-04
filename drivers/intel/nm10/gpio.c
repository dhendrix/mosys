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

#include "drivers/intel/ich_generic.h"

#include "lib/math.h"

#include "mosys/platform.h"

int nm10_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	return ich_read_gpio(intf, ICH7, gpio);
}

int nm10_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state)
{
	return ich_set_gpio(intf, ICH7, gpio, state);
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
	int bank0_gpios[] = { 0,  1,  2,  3,  4,  5,  6,  7,
	                      8,  9,  10, 11, 12, 13, 14, 15,
			      16, 17, 18, 19, 20, 21, 22, 23,
			      24, 25, 26, 27, 28, 29, 30, 31
	};
	int bank1_gpios[] = { 33, 34, 36, 38, 39, 48, 49 };
	int i, rc = 0;

	for (i = 0; i < ARRAY_SIZE(bank1_gpios); i++)
		bank1_gpios[i] = ICH_GPIO_PORT1_TO_PIN(bank1_gpios[i]);

	rc |= ich_gpio_list(intf, ICH7, 0,
	                    &bank0_gpios[0], ARRAY_SIZE(bank0_gpios));
	rc |= ich_gpio_list(intf, ICH7, 1,
	                    &bank1_gpios[0], ARRAY_SIZE(bank1_gpios));

	return rc;
}
