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
 * gpio.c: GPIO functions generic to all platforms
 */

#include "mosys/kv_pair.h"
#include "mosys/log.h"

#include "drivers/gpio.h"

/*
 * kv_pair_print_gpio  -  print gpio info and state
 *
 * @gpio:	gpio data
 * @state:	gpio state
 */
void kv_pair_print_gpio(struct gpio_map *gpio, int state)
{
	struct kv_pair *kv;

	kv = kv_pair_new();
	kv_pair_add(kv, "device", gpio->devname);
	kv_pair_fmt(kv, "id", "GPIO%02u", gpio->id);

	switch (gpio->type) {
	case GPIO_IN:
		kv_pair_add(kv, "type", "IN");
		break;
	case GPIO_OUT:
		kv_pair_add(kv, "type", "OUT");
		break;
	default:
		lprintf(LOG_DEBUG, "Invalid GPIO type %d\n", gpio->type);
		kv_pair_free(kv);
		return;
	}

	kv_pair_fmt(kv, "state", "%d", state);
	kv_pair_add(kv, "name", gpio->name);

	kv_pair_print(kv);
	kv_pair_free(kv);
}
