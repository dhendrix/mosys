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

#include "series5.h"

#define GPIO_NM10	0

/* gpio number, in/out, device, port, pin, negate, devname, name */
static struct gpio_map platform_gpio_map[] = {
	/* Note: Debug connector pins (CHP_*) may be bi-directional */
	/* id, type,    dev,          port, pin, neg, devname, name */
	{   0, GPIO_OUT, GPIO_NM10,    0,     0,   1, "NM10", "BM_BUSY#" },
	{   1, GPIO_IN,  GPIO_NM10,    0,     1,   0, "NM10", "KBC3_DVP_MODE" },
	{   6, GPIO_IN,  GPIO_NM10,    0,     6,   1, "NM10", "KBC3_RUNSCI#" },
	{   7, GPIO_OUT, GPIO_NM10,    0,     7,   0, "NM10", "SIM3_C4DET" },	/* neg? */
	{   8, GPIO_OUT, GPIO_NM10,    0,     8,   1, "NM10", "CHP3_MINICARD_PWRON#" },
	{   9, GPIO_IN,  GPIO_NM10,    0,     9,   0, "NM10", "CHP3_LS_INT" },
//	{  10, GPIO_,  GPIO_NM10,    0,    10,   X,  "NM10", "CHP3_DEBUG10" },
	{  12, GPIO_IN,  GPIO_NM10,    0,    12,   1, "NM10", "KBC3_WAKESCI#" },
//	{  13, GPIO_,  GPIO_NM10,    0,    13,   X,  "NM10", "CHP3_DEBUG13" },
	{  14, GPIO_IN,  GPIO_NM10,    0,    14,   0, "NM10", "CHP_TP_INT" },
	{  24, GPIO_IN,  GPIO_NM10,    0,    24,   0, "NM10", "CHP3_BOARD_ID0" },
//	{  25, GPIO_,   GPIO_NM10,   0,    25,   X, "NM10", "" }, /* dmi ac coupling mode */
	{  26, GPIO_IN,  GPIO_NM10,    0,    26,   0, "NM10", "CHP3_BOARD_ID1" },
	{  27, GPIO_IN,  GPIO_NM10,    0,    27,   1, "NM10", "CHP3_RFOFF_WLAN#" },
	{  28, GPIO_IN,  GPIO_NM10,    0,    28,   1, "NM10", "CHP3_RFOFF_HSDPA#" },
	{  33, GPIO_IN,  GPIO_NM10,    1,     1,   1, "NM10", "CHP3_RFOFF_BT#" },
	{  34, GPIO_IN,  GPIO_NM10,    1,     2,   0, "NM10", "KBC3_SPI_WP" },
	{  36, GPIO_IN,  GPIO_NM10,    1,     4,   0, "NM10", "BOARD_CONFIG" },
	{  38, GPIO_IN,  GPIO_NM10,    1,     6,   1, "NM10", "CHP3_REC_MODE#" },
	{  49, GPIO_IN,  GPIO_NM10,    1,    17,   0, "NM10", "CPU1_PWRGD" },

	{   0,       0,          0,    0,     0,   0,   NULL, NULL } /* end */
};

/*
 * samsung_series5_gpio_read  -  read level for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns pointer to GPIO map entry if successful
 * returns NULL to indicate failure
 */
static int samsung_series5_gpio_read(struct platform_intf *intf,
                                    struct gpio_map *gpio)
{
	int ret = 0;

	switch (gpio->dev) {
	case GPIO_NM10:
		ret = nm10_read_gpio(intf, gpio);
		break;
	default:
		ret = -1;
	}

	return ret;
}

/*
 * samsung_series5_gpio_map  -  get mapping info for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns pointer to GPIO map entry if successful
 * returns NULL to indicate failure
 */
struct gpio_map *samsung_series5_gpio_map(struct platform_intf *intf,
                                         const char *name)
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
 * samsung_series5_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int samsung_series5_gpio_list(struct platform_intf *intf)
{
	int i;

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		int state = 0;

		switch (platform_gpio_map[i].dev) {
		case GPIO_NM10:
			state = nm10_read_gpio(intf, &platform_gpio_map[i]);
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
 * samsung_series5_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int samsung_series5_gpio_set(struct platform_intf *intf,
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
		default:
			return -1;
		}
		break;
	}

	return ret;
}

struct gpio_cb samsung_series5_gpio_cb = {
	.read	= samsung_series5_gpio_read,
	.map	= samsung_series5_gpio_map,
	.list	= samsung_series5_gpio_list,
	.set	= samsung_series5_gpio_set,
};
