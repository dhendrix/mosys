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
#include "mosys/string.h"
#include "mosys/log.h"
#include "mosys/kv_pair.h"

#include "intf/pci.h"
#include "intf/io.h"

#include "pinetrail.h"

#define GPIO_NM10	0
#define GPIO_NPCE781	1

/* gpio number, in/out, device, port, pin, negate, devname, name */
struct gpio_map agz_pinetrail_gpio_map[] = {
	/* FIXME: double check NM10 pins w14, w16, h19, m17 */
	/* id, type,    dev,          port, pin, neg, devname,   name */
	{  3, GPIO_IN,  GPIO_NPCE781, 0,    95,   1,  "NPCE781", "NBSWON#" },

	{ 75, GPIO_IN,  GPIO_NPCE781, 0,    82,   0,  "NPCE781", "recovery_mode" },
//	{  6, GPIO_IN,  GPIO_NM10,    0,     0,  16,  "NM10", "recovery_mode" },
	{  6, GPIO_IN,  GPIO_NM10,    0,     6,  16,  "NM10", "recovery_mode" },

	{ 94, GPIO_IN,  GPIO_NPCE781, 0,    101,  0,  "NPCE781", "developer_mode" },
//	{  7, GPIO_IN,  GPIO_NM10,    0,     14,  0,  "NM10", "developer_mode" },
	{  7, GPIO_IN,  GPIO_NM10,    0,      7,  0,  "NM10", "developer_mode" },

	{  6, GPIO_IN,  GPIO_NPCE781, 0,     93,  1,  "NPCE781", "LID#" },
//	{  9, GPIO_IN,  GPIO_NM10,    0,     19,  1,  "NM10", "LID#" },
	{  9, GPIO_IN,  GPIO_NM10,    0,      9,  1,  "NM10", "LID#" },

	{ 77, GPIO_IN,  GPIO_NPCE781, 0,     84,  0,  "NPCE781", "manufacturing_mode" },
//	{ 10, GPIO_IN,  GPIO_NM10,    0,     17,  0,  "NM10", "manufacturing_mode" },
	{ 10, GPIO_IN,  GPIO_NM10,    0,     10,  0,  "NM10", "manufacturing_mode" },

#if 0
	/* FIXME: double check all these pins... */
	{ 24, GPIO_OUT,  GPIO_NM10,   0,      3,  0,   "NM10", "DBG0" },
	{ 26, GPIO_OUT,  GPIO_NM10,   0,     19,  0,   "NM10", "DBG1" },
	{ 27, GPIO_OUT,  GPIO_NM10,   0,     20,  0,   "NM10", "DBG2" },
	{ 28, GPIO_OUT,  GPIO_NM10,   0,     22,  0,   "NM10", "DBG3" },
	{ 33, GPIO_OUT,  GPIO_NM10,   0,     14,  0,   "NM10", "DBG4" },
	{ 34, GPIO_OUT,  GPIO_NM10,   0,      1,  0,   "NM10", "DBG5" },
	{ 38, GPIO_IN,   GPIO_NM10,   0,     23,  0,   "NM10", "DBG6" },
	{ 39, GPIO_IN,   GPIO_NM10,   0,     24,  0,   "NM10", "DBG7" },
#endif

	{ 0,  0,        0,               0, 0,  0, NULL,   NULL } /* end */
};

static struct gpio_map *platform_gpio_map = agz_pinetrail_gpio_map;

static uint16_t nm10_get_gpio_base(struct platform_intf *intf)
{
	static uint16_t nm10_gpio_base = 0;

	/* FIXME: replace this with a call to nm10_get_gpio_base */
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

/*
 * nm10_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
static int nm10_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
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

/*
 * nm10_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
static int nm10_set_gpio(struct platform_intf *intf,
                         struct gpio_map *gpio, int state)
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
 * kv_pair_print_gpio  -  print gpio info and state
 *
 * @gpio:	gpio data
 * @state:	gpio state
 */
static void kv_pair_print_gpio(struct gpio_map *gpio, int state)
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

/*
 * agz_pinetrail_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int agz_pinetrail_gpio_list(struct platform_intf *intf)
{
	int i;

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		int state = 0;

		switch (platform_gpio_map[i].dev) {
		case GPIO_NM10:
			state = nm10_read_gpio(intf, &platform_gpio_map[i]);
			break;
		case GPIO_NPCE781:
			/* FIXME: implement this */
//			state = npce781_read_gpio(intf, &platform_gpio_map[i]);
			state = -1;
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
 * agz_pinetrail_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int agz_pinetrail_gpio_set(struct platform_intf *intf,
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
		case GPIO_NM10:
			ret = nm10_set_gpio(intf, gpio, state);
			break;
#if 0
		case GPIO_NPCE781:
			ret = npce781_set_gpio(intf, gpio, state);
			break;
#endif
		default:
			return -1;
		}
		break;
	}

	return ret;
}

struct gpio_cb agz_pinetrail_gpio_cb = {
	.list	= agz_pinetrail_gpio_list,
	.set	= agz_pinetrail_gpio_set,
};
