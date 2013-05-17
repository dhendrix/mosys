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
#include <time.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/samsung/exynos_generic.h"
#include "drivers/samsung/exynos5250/gpio.h"
#include "drivers/samsung/exynos5420/gpio.h"

#include "intf/mmio.h"

#include "lib/math.h"

/* "safe" value for allowing a GPIO settle after changing configuration */
#define GPIO_DELAY_NS 5000	/* nanoseconds */

/* gpio struct must have at minimum valid port and pin values */
int exynos_read_gpio(struct platform_intf *intf,
		     const struct exynos_gpio_bank *banks,
		     struct gpio_map *gpio)
{
	const struct exynos_gpio_bank *bank;
	uint32_t dat;

	MOSYS_CHECK(banks);

	bank = &banks[gpio->port];

	if (mmio_read32(intf, bank->baseaddr + 0x04, &dat) < 0)
		return -1;

	return dat & (1 << gpio->pin) ? 1 : 0;
}

/* TODO: this could be made more generic with better GPIO config methods */
int exynos_read_gpio_mvl(struct platform_intf *intf,
			 const struct exynos_gpio_bank *banks,
			 struct gpio_map *gpio)
{
	const struct exynos_gpio_bank *bank;
	struct exynos_gpio_regs *regs;
	int vpu, vpd, ret;
	uint32_t dat, con, pud, pud_orig;
	struct timespec rem, req = { .tv_sec = 0, .tv_nsec = GPIO_DELAY_NS };

	MOSYS_CHECK(banks);

	bank = &banks[gpio->port];
	regs = (struct exynos_gpio_regs *)bank->baseaddr;

	/* check that GPIO is configured as an input (con[n] == 0) */
	if (mmio_read32(intf, (off_t)&regs->con, &con) < 0)
		return -1;
	con &= 0xf << (gpio->pin * 4);
	if (con) {
		lprintf(LOG_DEBUG, "%s: GPIO pin %d is not an input\n",
			__func__, gpio->pin);
		return -1;
	}

	if (mmio_read32(intf, (off_t)&regs->pud, &pud_orig) < 0)
		return -1;
	pud = pud_orig;

	/* get value when internal pull-up is enabled */
	pud |= 0x3 << (gpio->pin * 2);
	if (mmio_write32(intf, (off_t)&regs->pud, pud) < 0)
		return -1;
	if (nanosleep(&req, &rem) < 0)
		return -1;

	if (mmio_read32(intf, (off_t)&regs->dat, &dat) < 0)
		return -1;
	vpu = dat & (1 << gpio->pin) ? 1 : 0;

	/* get value when internal pull-down is enabled */
	pud &= ~(0x3 << (gpio->pin * 2));
	pud |= 0x1 << (gpio->pin * 2);
	if (mmio_write32(intf, (off_t)&regs->pud, pud) < 0)
		return -1;
	if (nanosleep(&req, &rem) < 0)
		return -1;

	if (mmio_read32(intf, (off_t)&regs->dat, &dat) < 0)
		return -1;
	vpd = dat & (1 << gpio->pin) ? 1 : 0;

	lprintf(LOG_DEBUG, "Pin %d, Vpu: %d, Vpd: %d\n", gpio->pin, vpu, vpd);
	if (!vpu && !vpd) {
		ret = LOGIC_0;
	} else if (vpu && vpd) {
		ret = LOGIC_1;
	} else if (vpu && !vpd) {
		ret = LOGIC_Z;
	} else {
		lprintf(LOG_DEBUG, "%s: detected unvalid logic state\n");
		return -1;
	}

	/* restore original pull-up/down value */
	if (mmio_write32(intf, (off_t)&regs->pud, pud_orig) < 0)
		return -1;
	if (nanosleep(&req, &rem) < 0)
		return -1;
	return ret;
}

int exynos_set_gpio(struct platform_intf *intf,
		    const struct exynos_gpio_bank *banks,
		    struct gpio_map *gpio, int state)
{
	const struct exynos_gpio_bank *bank;
	uint32_t dat;

	MOSYS_CHECK(banks);
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
int exynos_gpio_list(struct platform_intf *intf,
		     const struct exynos_gpio_bank *banks)
{
	const struct exynos_gpio_bank *bank;
	struct gpio_map gpio;
	int i;

	MOSYS_CHECK(banks);

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

			state = exynos_read_gpio(intf, banks, &gpio);
			if (state < 0)
				return -1;

			if (kv_pair_print_gpio(&gpio, state) < 0)
				return -1;
		}
	}

	return 0;
}
