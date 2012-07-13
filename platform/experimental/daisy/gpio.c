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

#include "mosys/platform.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/samsung/exynos5.h"

#include "intf/mmio.h"

#include "lib/string.h"

#include "daisy.h"

#define GPIO_SOC	0

#define BOARD_ID0	"ID_Bit0"
#define BOARD_ID1	"ID_Bit1"

/* gpio number, in/out, device, port, pin, negate, devname, name */
static struct gpio_map snow_gpio_map[] = {
	{   4, GPIO_IN,  GPIO_SOC, EXYNOS5_GPA0,  4, 0, "SOC", BOARD_ID0 },
	{   5, GPIO_IN,  GPIO_SOC, EXYNOS5_GPA0,  5, 0, "SOC", BOARD_ID1 },
	{   0,       0,         0,       0,  0,   0, NULL, NULL   }
};

/*
 * daisy_gpio_read  -  read level for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns GPIO level (0 or 1) to indicate success
 * returns <0 to indicate failure
 */
static int daisy_gpio_read(struct platform_intf *intf, struct gpio_map *gpio)
{
	int ret = 0;

	switch (gpio->dev) {
	case GPIO_SOC:
		ret = exynos5_read_gpio(intf, gpio);
		break;
	default:
		ret = -1;
	}

	return ret;
}

/*
 * daisy_gpio_map  -  get mapping info for one GPIO
 *
 * @intf:	platform interface
 * @name:	name of GPIO to get state for
 *
 * returns pointer to GPIO map entry if successful
 * returns NULL to indicate failure
 */
struct gpio_map *daisy_gpio_map(struct platform_intf *intf, const char *name)
{
	int i;
	struct gpio_map *gpio = NULL;

	for (i = 0; snow_gpio_map[i].name != NULL; i++) {
		struct gpio_map *tmp;

		tmp = &snow_gpio_map[i];

		/* look for GPIO by name */
		if (!strncmp(name, tmp->name, __minlen(name, tmp->name))) {
			gpio = tmp;
			break;
		}
	}

	return gpio;
}

/*
 * daisy_gpio_set  -  set state for one GPIO
 *
 * @intf:	platform interface
 * @name:	GPIO name
 * @state:	desired state 0|1
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int daisy_gpio_set(struct platform_intf *intf,
                          const char *name, int state)
{
	int i;
	int ret = 0;
	struct gpio_map *gpio;

	for (i = 0; snow_gpio_map[i].name != NULL; i++) {
		gpio = &snow_gpio_map[i];

		/* look for GPIO by name */
		if (strncmp(name, gpio->name, __minlen(name, gpio->name)))
			continue;

		/* can only set output GPIO */
		if (gpio->type != GPIO_OUT) {
			lprintf(LOG_ERR, "Unable to set input GPIO\n");
			return -1;
		}

		switch (gpio->dev) {
		case GPIO_SOC:
			ret = exynos5_set_gpio(intf, gpio, state);
			break;
		default:
			return -1;
		}
		break;
	}

	return ret;
}

struct gpio_cb daisy_gpio_cb = {
	.read	= daisy_gpio_read,
	.map	= daisy_gpio_map,
	.list	= exynos5_gpio_list,	/* generic listing for now */
	.set	= daisy_gpio_set,
};
