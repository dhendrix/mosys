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
 * gpio.c: GPIO functions for Exynos SoCs
 */

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/samsung/exynos_generic.h"
#include "drivers/samsung/exynos5.h"

#include "intf/mmio.h"

#include "lib/math.h"

static const struct exynos_gpio_bank *get_banks(enum exynos_generation gen)
{
	const struct exynos_gpio_bank *banks = NULL;

	switch (gen) {
	case EXYNOS5:
		banks = exynos5_gpio_banks;
		break;
	default:
		break;
	}

	return banks;
}

/* gpio struct must have at minimum valid port and pin values */
int exynos_read_gpio(struct platform_intf *intf,
		     enum exynos_generation gen, struct gpio_map *gpio)
{
	const struct exynos_gpio_bank *bank, *banks;
	uint32_t dat;

	banks = get_banks(gen);
	bank = &banks[gpio->port];

	if (mmio_read32(intf, bank->baseaddr + 0x04, &dat) < 0)
		return -1;

	return dat & (1 << gpio->pin) ? 1 : 0;
}

int exynos_set_gpio(struct platform_intf *intf, enum exynos_generation gen,
		    struct gpio_map *gpio, int state)
{
	const struct exynos_gpio_bank *bank, *banks;
	uint32_t dat;

	banks = get_banks(gen);
	bank = &banks[gpio->port];

	if (mmio_read32(intf, bank->baseaddr + 0x04, &dat) < 0)
		return -1;

	if (state == 0)
		dat &= ~(1 << gpio->pin);
	else if (state == 1)
		dat |= 1 << gpio->pin;
	else
		return -1;

	if (mmio_write32(intf, bank->baseaddr + 0x04, dat) < 0)
		return -1;

	return 0;
}

/*
 * exynos_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int exynos_gpio_list(struct platform_intf *intf, enum exynos_generation gen)
{
	const struct exynos_gpio_bank *bank, *banks;
	struct gpio_map gpio;
	int i;

	banks = get_banks(gen);
	if (!banks)
		return -1;

	for (bank = banks, i = 0; bank && bank->name; bank++, i++) {
		int j;
		struct exynos_gpio_regs regs;
		uint32_t baseaddr = bank->baseaddr;

		if (mmio_read32(intf, baseaddr + 0x00, &regs.con) < 0)
			return -1;

		gpio.dev = 0;	/* assume 0 for generic listing */
		gpio.port = i;
		gpio.devname = "SOC";
		gpio.neg = 0;	/* assume 0 for generic listing */

		for (j = 0; j < 8; j++) {
			int tmp;
			uint32_t mask = __mask(j * 4 + 4, j * 4);
			char gpioname[16];
			int state;

			gpio.id = (i * 8) + j;

			tmp = (regs.con & mask) >> ctz(mask);
			if (tmp == 0)
				gpio.type = GPIO_IN;
			else if (tmp == 1)
				gpio.type = GPIO_OUT;
			else
				gpio.type = GPIO_ALT;

			gpio.pin = j;

			sprintf(gpioname, "%s_%d", bank->name, gpio.pin);
			gpio.name = gpioname;

			state = exynos_read_gpio(intf, EXYNOS5, &gpio);
			if (state < 0)
				return -1;

			kv_pair_print_gpio(&gpio, state);
		}
	}

	return 0;
}
