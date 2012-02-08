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
 */

#include "mosys/platform.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/intel/series6.h"

#include "intf/io.h"

#include "lib/string.h"

#include "lumpy.h"

#define GPIO_PCH	LUMPY_GPIO_PCH

/* gpio number, in/out, device, port, pin, negate, devname, name */
static struct gpio_map platform_gpio_map[] = {
	{  33, GPIO_IN,  GPIO_PCH,    1,     1,   1, "PCH", "GPIO33" },
	{  41, GPIO_IN,  GPIO_PCH,    1,     9,   1, "PCH", "GPIO41" },
	{  49, GPIO_IN,  GPIO_PCH,    1,    17,   1, "PCH", "GPIO49" },
	{   0,       0,         0,    0,     0,   0,  NULL, NULL     } /* end */
};

/*
 * lumpy_gpio_read  -  read level for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns GPIO level (0 or 1) to indicate success
 * returns <0 to indicate failure
 */
static int lumpy_gpio_read(struct platform_intf *intf, struct gpio_map *gpio)
{
	int ret = 0;

	switch (gpio->dev) {
	case GPIO_PCH:
		ret = series6_read_gpio(intf, gpio);
		break;
	default:
		ret = -1;
	}

	return ret;
}

/*
 * lumpy_gpio_map  -  get mapping info for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns pointer to GPIO map entry if successful
 * returns NULL to indicate failure
 */
struct gpio_map *lumpy_gpio_map(struct platform_intf *intf, const char *name)
{
	int i;
	struct gpio_map *gpio = NULL;

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		struct gpio_map *tmp;

		tmp = &platform_gpio_map[i];

		/* look for GPIO by name */
		if (!strncmp(name, tmp->name, __minlen(name, tmp->name))) {
			gpio = tmp;
			break;
		}
	}

	return gpio;
}

/*
 * lumpy_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int lumpy_gpio_list(struct platform_intf *intf)
{
	int i;

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		int state = 0;

		switch (platform_gpio_map[i].dev) {
		case GPIO_PCH:
			state = series6_read_gpio(intf, &platform_gpio_map[i]);
			break;
		default:
			return -1;
		}

		if (state < 0)
			continue;

		kv_pair_print_gpio(&platform_gpio_map[i], state);
	}

	return 0;
}

/*
 * lumpy_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int lumpy_gpio_set(struct platform_intf *intf,
                          const char *name, int state)
{
	int i;
	int ret = 0;
	struct gpio_map *gpio;

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		gpio = &platform_gpio_map[i];

		/* look for GPIO by name */
		if (strncmp(name, gpio->name, __minlen(name, gpio->name)))
			continue;

		/* can only set output GPIO */
		if (gpio->type != GPIO_OUT) {
			lprintf(LOG_ERR, "Unable to set input GPIO\n");
			return -1;
		}

		switch (gpio->dev) {
		case GPIO_PCH:
			ret = series6_set_gpio(intf, gpio, state);
			break;
		default:
			return -1;
		}
		break;
	}

	return ret;
}

struct gpio_cb lumpy_gpio_cb = {
	.read	= lumpy_gpio_read,
	.map	= lumpy_gpio_map,
	.list	= lumpy_gpio_list,
	.set	= lumpy_gpio_set,
};
