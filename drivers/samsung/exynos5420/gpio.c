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
 *
 * gpio.c: GPIO functions for Exynos5420
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/samsung/exynos_generic.h"
#include "drivers/samsung/exynos5420/gpio.h"

#define EXYNOS5420_GPIO_RIGHT	0x13400000
#define EXYNOS5420_GPIO_TOP	0x13410000
#define EXYNOS5420_GPIO_LEFT	0x14000000
#define EXYNOS5420_GPIO_BOTTOM	0x14010000

/* banks of GPIO registers corresponding to a port */
const struct exynos_gpio_bank exynos5420_gpio_banks[] = {
	/*
	 * Note 1: ETC registers are special, so they are not included here.
	 */

	/* base == EXYNOS5420_GPIO_RIGHT */
	{ "GPY7", EXYNOS5420_GPIO_RIGHT + 0x0000 },
	{ "GPX0", EXYNOS5420_GPIO_RIGHT + 0x0c00 },
	{ "GPX1", EXYNOS5420_GPIO_RIGHT + 0x0c20 },
	{ "GPX2", EXYNOS5420_GPIO_RIGHT + 0x0c40 },
	{ "GPX3", EXYNOS5420_GPIO_RIGHT + 0x0c60 },

	{ "GPC0", EXYNOS5420_GPIO_TOP + 0x0000 },
	{ "GPC1", EXYNOS5420_GPIO_TOP + 0x0020 },
	{ "GPC2", EXYNOS5420_GPIO_TOP + 0x0040 },
	{ "GPC3", EXYNOS5420_GPIO_TOP + 0x0060 },
	{ "GPC4", EXYNOS5420_GPIO_TOP + 0x0080 },
	{ "GPD1", EXYNOS5420_GPIO_TOP + 0x00a0 },
	{ "GPY0", EXYNOS5420_GPIO_TOP + 0x00c0 },
	{ "GPY1", EXYNOS5420_GPIO_TOP + 0x00e0 },
	{ "GPY2", EXYNOS5420_GPIO_TOP + 0x0100 },
	{ "GPY3", EXYNOS5420_GPIO_TOP + 0x0120 },
	{ "GPY4", EXYNOS5420_GPIO_TOP + 0x0140 },
	{ "GPY5", EXYNOS5420_GPIO_TOP + 0x0160 },
	{ "GPY6", EXYNOS5420_GPIO_TOP + 0x0180 },

	{ "GPE0", EXYNOS5420_GPIO_LEFT + 0x0000 },
	{ "GPE1", EXYNOS5420_GPIO_LEFT + 0x0020 },
	{ "GPF0", EXYNOS5420_GPIO_LEFT + 0x0040 },
	{ "GPF1", EXYNOS5420_GPIO_LEFT + 0x0060 },
	{ "GPG0", EXYNOS5420_GPIO_LEFT + 0x0080 },
	{ "GPG1", EXYNOS5420_GPIO_LEFT + 0x00a0 },
	{ "GPG2", EXYNOS5420_GPIO_LEFT + 0x00c0 },
	{ "GPJ4", EXYNOS5420_GPIO_LEFT + 0x00e0 },

	{ "GPA0", EXYNOS5420_GPIO_BOTTOM + 0x0000 },
	{ "GPA1", EXYNOS5420_GPIO_BOTTOM + 0x0020 },
	{ "GPA2", EXYNOS5420_GPIO_BOTTOM + 0x0040 },
	{ "GPB0", EXYNOS5420_GPIO_BOTTOM + 0x0060 },
	{ "GPB1", EXYNOS5420_GPIO_BOTTOM + 0x0080 },
	{ "GPB2", EXYNOS5420_GPIO_BOTTOM + 0x00a0 },
	{ "GPB3", EXYNOS5420_GPIO_BOTTOM + 0x00c0 },
	{ "GPB4", EXYNOS5420_GPIO_BOTTOM + 0x00e0 },
	{ "GPH0", EXYNOS5420_GPIO_BOTTOM + 0x0100 },

	{ "GPZ", 0x03860000 + 0x0000 },

	{ NULL },
};

int exynos5420_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	return exynos_read_gpio(intf, exynos5420_gpio_banks, gpio);
}

int exynos5420_read_gpio_mvl(struct platform_intf *intf, struct gpio_map *gpio)
{
	return exynos_read_gpio_mvl(intf, exynos5420_gpio_banks, gpio);
}

int exynos5420_set_gpio(struct platform_intf *intf,
		     struct gpio_map *gpio, int state)
{
	return exynos_set_gpio(intf, exynos5420_gpio_banks, gpio, state);
}

int exynos5420_gpio_list(struct platform_intf *intf)
{
	return exynos_gpio_list(intf, exynos5420_gpio_banks);
}
