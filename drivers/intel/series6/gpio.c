/*
 * Copyright (C) 2012 Google Inc.
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

#include "mosys/platform.h"

#include "drivers/intel/ich_generic.h"

#include "lib/math.h"

int series6_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	return ich_read_gpio(intf, ICH_6_SERIES, gpio);
}

int series6_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state)
{
	return ich_set_gpio(intf, ICH_6_SERIES, gpio, state);
}

/*
 * series6_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int series6_gpio_list(struct platform_intf *intf)
{
	int bank0_gpios[32], bank1_gpios[32];
	int bank2_gpios[] = { 64, 65, 66, 67, 68, 69, 70, 71,
	                      72, 73, 74, 75 };
	int i, rc = 0;

	/* GPIO banks 0 and 1 are fully implemented */
	for (i = 0; i < 32; i++) {
		bank0_gpios[i] = i;
		bank1_gpios[i] = i;
	}

	for (i = 0; i < ARRAY_SIZE(bank2_gpios); i++)
		bank2_gpios[i] = ICH_GPIO_PORT2_TO_PIN(bank2_gpios[i]);

	rc |= ich_gpio_list(intf, ICH_6_SERIES, 0,
	                    &bank0_gpios[0], ARRAY_SIZE(bank0_gpios));
	rc |= ich_gpio_list(intf, ICH_6_SERIES, 1,
	                    &bank1_gpios[0], ARRAY_SIZE(bank1_gpios));
	rc |= ich_gpio_list(intf, ICH_6_SERIES, 2,
	                    &bank2_gpios[0], ARRAY_SIZE(bank2_gpios));

	return rc;
}
