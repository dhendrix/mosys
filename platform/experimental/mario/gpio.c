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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "mosys/platform.h"
#include "mosys/log.h"
#include "mosys/kv_pair.h"

#include "drivers/gpio.h"

#include "drivers/intel/nm10.h"

#include "intf/pci.h"
#include "intf/io.h"

#include "lib/string.h"

#include "pinetrail.h"

#define GPIO_NM10	0

#if 0
/* gpio number, in/out, device, port, pin, negate, devname, name */
struct gpio_map mario_pinetrail_gpio_map[] = {
	/* id, type,    dev,          port, pin, neg, devname,   name */
	{ 0,  0,        0,            0, 0,  0, NULL,   NULL } /* end */
};
#endif

struct gpio_cb mario_pinetrail_gpio_cb = {
	.list	= nm10_gpio_list,
};
