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
 *
 * gpio.c: GPIO functions for Exynos5
 */

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/samsung/exynos_generic.h"
#include "drivers/samsung/exynos5.h"

#include "intf/mmio.h"

#define EXYNOS5_GPIO_BASE0	0x11400000
#define EXYNOS5_GPIO_BASE1	0x13400000
#define EXYNOS5_GPIO_BASE2	0x10d10000
#define EXYNOS5_GPIO_BASE3	0x03860000

/* banks of GPIO registers corresponding to a port */
const struct exynos_gpio_bank exynos5_gpio_banks[] = {
	/*
	 * Note 1: Keep in order of exynos5_gpio_port enum.
	 * Note 2: ETC registers are special, so they are not included here.
	 */

	/* base == EXYNOS_GPIO_BASE0 */
	{ "GPA0", EXYNOS5_GPIO_BASE0 + 0x0000 },
	{ "GPA1", EXYNOS5_GPIO_BASE0 + 0x0020 },
	{ "GPA2", EXYNOS5_GPIO_BASE0 + 0x0040 },

	{ "GPB0", EXYNOS5_GPIO_BASE0 + 0x0060 },
	{ "GPB1", EXYNOS5_GPIO_BASE0 + 0x0080 },
	{ "GPB2", EXYNOS5_GPIO_BASE0 + 0x00a0 },
	{ "GPB3", EXYNOS5_GPIO_BASE0 + 0x00c0 },

	{ "GPC0", EXYNOS5_GPIO_BASE0 + 0x00e0 },
	{ "GPC1", EXYNOS5_GPIO_BASE0 + 0x0100 },
	{ "GPC2", EXYNOS5_GPIO_BASE0 + 0x0120 },
	{ "GPC3", EXYNOS5_GPIO_BASE0 + 0x0140 },

	{ "GPD0", EXYNOS5_GPIO_BASE0 + 0x0160 },
	{ "GPD1", EXYNOS5_GPIO_BASE0 + 0x0180 },

	{ "GPY0", EXYNOS5_GPIO_BASE0 + 0x01a0 },
	{ "GPY1", EXYNOS5_GPIO_BASE0 + 0x01c0 },
	{ "GPY2", EXYNOS5_GPIO_BASE0 + 0x01e0 },
	{ "GPY3", EXYNOS5_GPIO_BASE0 + 0x0200 },
	{ "GPY4", EXYNOS5_GPIO_BASE0 + 0x0220 },
	{ "GPY5", EXYNOS5_GPIO_BASE0 + 0x0240 },
	{ "GPY6", EXYNOS5_GPIO_BASE0 + 0x0260 },

	{ "GPX0", EXYNOS5_GPIO_BASE0 + 0x0c00 },
	{ "GPX1", EXYNOS5_GPIO_BASE0 + 0x0c20 },
	{ "GPX2", EXYNOS5_GPIO_BASE0 + 0x0c40 },
	{ "GPX3", EXYNOS5_GPIO_BASE0 + 0x0c60 },

	/* base == EXYNOS_GPIO_BASE1 */
	{ "GPE0", EXYNOS5_GPIO_BASE1 + 0x0000 },
	{ "GPE1", EXYNOS5_GPIO_BASE1 + 0x0020 },

	{ "GPF0", EXYNOS5_GPIO_BASE1 + 0x0040 },
	{ "GPF1", EXYNOS5_GPIO_BASE1 + 0x0060 },

	{ "GPG0", EXYNOS5_GPIO_BASE1 + 0x0080 },
	{ "GPG1", EXYNOS5_GPIO_BASE1 + 0x00a0 },
	{ "GPG2", EXYNOS5_GPIO_BASE1 + 0x00c0 },

	{ "GPH0", EXYNOS5_GPIO_BASE1 + 0x00e0 },
	{ "GPH1", EXYNOS5_GPIO_BASE1 + 0x0100 },

	/* base == EXYNOS_GPIO_BASE2 */
	{ "GPV0", EXYNOS5_GPIO_BASE2 + 0x0000 },
	{ "GPV1", EXYNOS5_GPIO_BASE2 + 0x0020 },
	{ "GPV2", EXYNOS5_GPIO_BASE2 + 0x0060 },
	{ "GPV3", EXYNOS5_GPIO_BASE2 + 0x0080 },
	{ "GPV4", EXYNOS5_GPIO_BASE2 + 0x00c0 },

	/* base == EXYNOS_GPIO_BASE3 */
	{ "GPZ", EXYNOS5_GPIO_BASE3 + 0x0000 },

	{ NULL },
};

int exynos5_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	return exynos_read_gpio(intf, EXYNOS5, gpio);
}

int exynos5_set_gpio(struct platform_intf *intf,
		     struct gpio_map *gpio, int state)
{
	return exynos_set_gpio(intf, EXYNOS5, gpio, state);
}

int exynos5_gpio_list(struct platform_intf *intf)
{
	return exynos_gpio_list(intf, EXYNOS5);
}
