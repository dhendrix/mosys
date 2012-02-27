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

#include "chromia700.h"

#define GPIO_NM10	0
#define GPIO_NPCE781	1

/* gpio number, in/out, device, port, pin, negate, devname, name */
struct gpio_map acer_chromia700_gpio_map[] = {
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

static struct gpio_map *platform_gpio_map = acer_chromia700_gpio_map;

/*
 * acer_chromia700_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int acer_chromia700_gpio_list(struct platform_intf *intf)
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
 * acer_chromia700_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int acer_chromia700_gpio_set(struct platform_intf *intf,
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

struct gpio_cb acer_chromia700_gpio_cb = {
	.list	= acer_chromia700_gpio_list,
	.set	= acer_chromia700_gpio_set,
};
