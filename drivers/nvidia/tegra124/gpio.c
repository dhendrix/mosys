/*
 * Copyright 2014, Google Inc.
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

#include <unistd.h>
#include "drivers/nvidia/tegra124/gpio.h"
#include "intf/mmio.h"
#include "lib/math.h"
#include "mosys/platform.h"

static const struct gpio_bank *gpio_banks = (void *)TEGRA_GPIO_BASE;
static u32 *pinmux_regs = (void *)TEGRA_APB_PINMUX_BASE;

void gpio_write_port(struct platform_intf *intf, int index,
		size_t offset, u32 mask, u32 value)
{
	u32 reg, new_reg;
	int bank = index / GPIO_GPIOS_PER_BANK;
	int port = (index - bank * GPIO_GPIOS_PER_BANK) / GPIO_GPIOS_PER_PORT;

	mmio_read32(intf, (u32)((u8 *)&gpio_banks[bank] + offset +
	            port * sizeof(u32)), &reg);
	new_reg  = (reg & ~mask) | (value & mask);

	if (new_reg != reg) {
		mmio_write32(intf, (u32)((u8 *)&gpio_banks[bank] + offset +
		port * sizeof(u32)), new_reg);
	}
}

u32 gpio_read_port(struct platform_intf *intf, int index, size_t offset)
{
	u32 reg;
	int bank = index / GPIO_GPIOS_PER_BANK;
	int port = (index - bank * GPIO_GPIOS_PER_BANK) / GPIO_GPIOS_PER_PORT;

	mmio_read32(intf, (u32)((u8 *)&gpio_banks[bank] + offset +
	            port * sizeof(u32)), &reg);
	return reg;
}

void gpio_set_int_enable(struct platform_intf *intf,
				gpio_t gpio, int enable)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	gpio_write_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
			offsetof(struct gpio_bank, int_enable),
			1 << bit, enable ? (1 << bit) : 0);
}

void gpio_set_out_enable(struct platform_intf *intf,
		gpio_t gpio, int enable)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	gpio_write_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
			offsetof(struct gpio_bank, out_enable),
			1 << bit, enable ? (1 << bit) : 0);
}

void gpio_set_mode(struct platform_intf *intf,
		gpio_t gpio, enum gpio_mode mode)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	gpio_write_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
			offsetof(struct gpio_bank, config),
			1 << bit, mode ? (1 << bit) : 0);
}

void pinmux_set_config(struct platform_intf *intf,
		int pin_index, uint32_t config)
{
	mmio_write32(intf, (u32)&pinmux_regs[pin_index], config);
}

void gpio_input(struct platform_intf *intf, gpio_t gpio, u32 pull)
{
	u32 pinmux_config = PINMUX_INPUT_ENABLE | pull;

	gpio_set_int_enable(intf, gpio, 0);
	gpio_set_out_enable(intf, gpio, 0);
	gpio_set_mode(intf, gpio, GPIO_MODE_GPIO);
	pinmux_set_config(intf, gpio >> GPIO_PINMUX_SHIFT, pinmux_config);
}

void gpio_input_pullup(struct platform_intf *intf, gpio_t gpio)
{
	gpio_input(intf, gpio, PINMUX_PULL_UP);
}

void gpio_input_pulldown(struct platform_intf *intf, gpio_t gpio)
{
	gpio_input(intf, gpio, PINMUX_PULL_DOWN);
}

int gpio_get_in_value(struct platform_intf *intf, gpio_t gpio)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	u32 port = gpio_read_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
				  offsetof(struct gpio_bank, in_value));
	return (port & (1 << bit)) != 0;
}

int gpio_get_in_tristate_values(struct platform_intf *intf,
		gpio_t gpio[], int num_gpio, int value[])
{
	/*
	 * GPIOs which are tied to stronger external pull up or pull down
	 * will stay there regardless of the internal pull up or pull
	 * down setting.
	 *
	 * GPIOs which are floating will go to whatever level they're
	 * internally pulled to.
	 */

	int temp;
	int index;

	/* Enable internal pull up */
	for (index = 0; index < num_gpio; ++index)
		gpio_input_pullup(intf, gpio[index]);

	/* Wait until signals become stable */
	usleep(10);

	/* Get gpio values at internal pull up */
	for (index = 0; index < num_gpio; ++index)
		value[index] = gpio_get_in_value(intf, gpio[index]);

	/* Enable internal pull down */
	for (index = 0; index < num_gpio; ++index)
		gpio_input_pulldown(intf, gpio[index]);

	/* Wait until signals become stable */
	usleep(10);

	/*
	 * Get gpio values at internal pull down.
	 * Compare with gpio pull up value and then
	 * determine a gpio final value/state:
	 *  0: pull down
	 *  1: pull up
	 *  2: floating
	 */
	for (index = 0; index < num_gpio; ++index) {
		temp = gpio_get_in_value(intf, gpio[index]);
		value[index] = ((value[index] ^ temp) << 1) | temp;
	}

	/* Disable pull up / pull down to conserve power */
	for (index = 0; index < num_gpio; ++index)
		gpio_input(intf, gpio[index], PINMUX_PULL_NONE);

	return 0;
}
