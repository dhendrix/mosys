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
 */

#ifndef MOSYS_DRIVERS_GPIO_H__
#define MOSYS_DRIVERS_GPIO_H__

enum gpio_types {
	GPIO_IN,
	GPIO_OUT,
};

struct gpio_map {
	int id;			/* gpio number */
	enum gpio_types type;	/* input/output */
	int dev;		/* device identifier */
	int port;		/* port in device */
	int pin;		/* pin in port in device */
	int neg;		/* pin is negated */
	const char *devname;	/* device name */
	const char *name;	/* gpio name */
};

/*
 * kv_pair_print_gpio  -  print gpio info and state
 *
 * @gpio:	gpio data
 * @state:	gpio state
 */
extern void kv_pair_print_gpio(struct gpio_map *gpio, int state);

#endif /* MOSYS_DRIVERS_GPIO_H__ */
