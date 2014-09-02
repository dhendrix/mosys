/*
 * Copyright 2013, Google Inc.
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

#include "mosys/platform.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/intel/lynxpoint_lp.h"
#include "mosys/kv_pair.h"

#include "intf/io.h"

#include "lib/string.h"

#include "samus.h"
#include "drivers/intel/lpss_generic.h"
#include "mosys/alloc.h"

#define num_of_gpio 95

/* gpio number, in/out, device, port, pin, negate, devname, name */
/* The last data in this array is NULL */
static struct gpio_map platform_gpio_map[num_of_gpio + 1];
static char pch_str[4] = "PCH";

static void setup_gpio(struct platform_intf *intf, struct gpio_reg *reg)
{
	uint8_t i = 0;
	char *tmp;

	for (i = 0; i < num_of_gpio; i++) {
		platform_gpio_map[i].id = i;
		platform_gpio_map[i].type = GPIO_IN;
		platform_gpio_map[i].dev = 0;
		platform_gpio_map[i].port = 0;
		platform_gpio_map[i].pin = i % 32;
		platform_gpio_map[i].neg = 0;
		platform_gpio_map[i].devname = pch_str;
		tmp = mosys_malloc(7);
		sprintf(tmp, "GPIO%02d", i);
		platform_gpio_map[i].name = tmp;
		lynxpoint_lp_read_gpio_attributes(intf, &platform_gpio_map[i],
						  &reg[i]);
	}
	platform_gpio_map[num_of_gpio + 1].name = NULL;
}

static void free_resource_for_string_in_gpio_map(void)
{
	int i;
	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		free((void *)platform_gpio_map[i].name);
	}
}
/*
 * samus_gpio_read  -  read level for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns GPIO level (0 or 1) to indicate success
 * returns <0 to indicate failure
 */
static int samus_gpio_read(struct platform_intf *intf, struct gpio_map *gpio)
{
	return lynxpoint_lp_read_gpio(intf, gpio);
}

/*
 * samus_gpio_map  -  get mapping info for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns pointer to GPIO map entry if successful
 * returns NULL to indicate failure
 */
struct gpio_map *samus_gpio_map(struct platform_intf *intf, const char *name)
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
 * samus_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int samus_gpio_list(struct platform_intf *intf)
{
	int i, ret;
	struct gpio_reg reg[num_of_gpio];
	setup_gpio(intf, reg);

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		ret = lynxpoint_lp_list_gpio_attributes(&platform_gpio_map[i],
							&reg[i]);
		if(ret < 0)
			return -1;
	}
	free_resource_for_string_in_gpio_map();
	return 0;
}

/*
 * samus_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int samus_gpio_set(struct platform_intf *intf,
                          const char *name, int state)
{
	int i;
	int ret = 0;
	struct gpio_map *gpio;
	struct gpio_reg reg[num_of_gpio];

	setup_gpio(intf, reg);

	for (i = 0; platform_gpio_map[i].name != NULL; i++) {
		gpio = &platform_gpio_map[i];

		/* look for GPIO by name */
		if (strncmp(name, gpio->name, __minlen(name, gpio->name)))
			continue;

		ret = lynxpoint_lp_set_gpio(intf, gpio, state);
		break;
	}
	free_resource_for_string_in_gpio_map();
	return ret;
}

struct gpio_cb samus_gpio_cb = {
	.read	= samus_gpio_read,
	.map	= samus_gpio_map,
	.list	= samus_gpio_list,
	.set	= samus_gpio_set,
};
