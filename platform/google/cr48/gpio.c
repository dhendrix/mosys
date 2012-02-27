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

#include "cr48.h"

#define GPIO_NM10	0
#define GPIO_IT8500	1

/* gpio number, in/out, device, port, pin, negate, devname, name */
struct gpio_map google_cr48_gpio_map[] = {
	/* id, type,    dev,          port, pin, neg, devname,   name */
	{  0, GPIO_OUT, GPIO_NM10,    0,    0,   1,  "NM10", "BM_BUSY#" },
	{  6, GPIO_IN,  GPIO_NM10,    0,    6,   1,  "NM10", "EC_SCI#" },
	{  7, GPIO_OUT, GPIO_NM10,    0,    7,   1,  "NM10", "SIM_CD#" },
	{  8, GPIO_OUT, GPIO_NM10,    0,    8,   0,  "NM10", "MC1_DISABLE" },
	{  9, GPIO_OUT, GPIO_NM10,    0,    9,   0,  "NM10", "PCH_GPIO9" },
	{ 10, GPIO_OUT, GPIO_NM10,    0,   10,   0,  "NM10", "PCH_GPIO10" },
	{ 11, GPIO_OUT, GPIO_NM10,    0,   11,   1,  "NM10", "SMBALERT#" },
	{ 12, GPIO_OUT, GPIO_NM10,    0,   12,   1,  "NM10", "PCH_GPIO12" },
	{ 13, GPIO_IN,  GPIO_NM10,    0,   13,   1,  "NM10", "PCH_GPIO13" },
	{ 14, GPIO_OUT, GPIO_NM10,    0,   14,   1,  "NM10", "PCH_GPIO14" },
	{ 15, GPIO_OUT, GPIO_NM10,    0,   15,   1,  "NM10", "PCH_GPIO15" },
	{ 24, GPIO_OUT, GPIO_NM10,    0,   24,   1,  "NM10", "PCH_GPIO24" },
	{ 26, GPIO_OUT, GPIO_NM10,    0,   26,   1,  "NM10", "PCH_GPIO26" },
	{ 27, GPIO_OUT, GPIO_NM10,    0,   27,   1,  "NM10", "WLAN_DISABLE#" },
	{ 28, GPIO_OUT, GPIO_NM10,    0,   28,   1,  "NM10", "WWAN_DISABLE#" },
	{ 33, GPIO_OUT, GPIO_NM10,    1,   1,    1,  "NM10", "BT_DISABLE#" },
	{ 34, GPIO_OUT, GPIO_NM10,    1,   2,    1,  "NM10", "SPI_WP" },
	{ 38, GPIO_IN,  GPIO_NM10,    1,   6,    1,  "NM10", "REC_MODE#" },
	{ 39, GPIO_OUT, GPIO_NM10,    1,   7,    1,  "NM10", "LS_TP_INT" },
	{ 49, GPIO_OUT, GPIO_NM10,    1,   17,   1,  "NM10", "H_PWRGD" },

	{ 0,  0,        0,               0, 0,  0, NULL,   NULL } /* end */
};

static struct gpio_map *platform_gpio_map = google_cr48_gpio_map;

/*
 * google_cr48_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int google_cr48_gpio_list(struct platform_intf *intf)
{
	int i;

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		int state = 0;

		switch (platform_gpio_map[i].dev) {
		case GPIO_NM10:
			state = nm10_read_gpio(intf, &platform_gpio_map[i]);
			break;
		case GPIO_IT8500:
			/* FIXME: implement this */
//			state = it8500_read_gpio(intf, &platform_gpio_map[i]);
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
 * google_cr48_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int google_cr48_gpio_set(struct platform_intf *intf,
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
		case GPIO_IT8500:
			ret = it8500_set_gpio(intf, gpio, state);
			break;
#endif
		default:
			return -1;
		}
		break;
	}

	return ret;
}

struct gpio_cb google_cr48_gpio_cb = {
	.list	= google_cr48_gpio_list,
	.set	= google_cr48_gpio_set,
};
