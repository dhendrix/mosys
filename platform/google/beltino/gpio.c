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

#include "intf/io.h"

#include "lib/string.h"

#include "beltino.h"

/* gpio number, in/out, device, port, pin, negate, devname, name */
static struct gpio_map platform_gpio_map[] = {
	{  13,  GPIO_IN,    0,    0,    13,   0, "PCH", "GPIO13" },
	{   9,  GPIO_IN,    0,    0,     9,   0, "PCH", "GPIO09" },
	{  47,  GPIO_IN,    0,    1,    15,   0, "PCH", "GPIO47" },
	{  67,  GPIO_IN,    0,    2,     3,   0, "PCH", "GPIO67" },
	{  68,  GPIO_IN,    0,    2,     4,   0, "PCH", "GPIO68" },
	{  69,  GPIO_IN,    0,    2,     5,   0, "PCH", "GPIO69" },
	{   0,        0,    0,    0,     0,   0,  NULL, NULL     } /* end */
};

/*
 * beltino_gpio_read  -  read level for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns GPIO level (0 or 1) to indicate success
 * returns <0 to indicate failure
 */
static int beltino_gpio_read(struct platform_intf *intf, struct gpio_map *gpio)
{
	return lynxpoint_lp_read_gpio(intf, gpio);
}

/*
 * beltino_gpio_map  -  get mapping info for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns pointer to GPIO map entry if successful
 * returns NULL to indicate failure
 */
struct gpio_map *beltino_gpio_map(struct platform_intf *intf, const char *name)
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
 * beltino_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int beltino_gpio_list(struct platform_intf *intf)
{
	return lynxpoint_lp_gpio_list(intf);
}

/*
 * beltino_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int beltino_gpio_set(struct platform_intf *intf,
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

		ret = lynxpoint_lp_set_gpio(intf, gpio, state);
		break;
	}

	return ret;
}

struct gpio_cb beltino_gpio_cb = {
	.read	= beltino_gpio_read,
	.map	= beltino_gpio_map,
	.list	= beltino_gpio_list,
	.set	= beltino_gpio_set,
};
