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

#include <stdlib.h>
#include <unistd.h>

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/google/cros_ec.h"

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "nyan.h"
#include "intf/mmio.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define TEGRA_GPIO_BASE 0x6000D000
#define TEGRA_APB_PINMUX_BASE 0x70003000

static const struct gpio_bank *gpio_banks = (void *)TEGRA_GPIO_BASE;
static u32 *pinmux_regs = (void *)TEGRA_APB_PINMUX_BASE;

const char *nyan_id_list[] = {
	"google,nyan",
	NULL,
};

const char *nyan_big_id_list[] = {
	"google,nyan-big",
	NULL,
};

const char *nyan_blaze_id_list[] = {
	"google,nyan-blaze-rev5",
	"google,nyan-blaze-rev4",
	"google,nyan-blaze-rev3",
	"google,nyan-blaze-rev2",
	"google,nyan-blaze-rev1",
	"google,nyan-blaze-rev0",
	NULL,
};

const char *nyan_kitty_id_list[] = {
	"google,nyan-kitty-rev5",
	"google,nyan-kitty-rev4",
	"google,nyan-kitty-rev3",
	"google,nyan-kitty-rev2",
	"google,nyan-kitty-rev1",
	"google,nyan-kitty-rev0",
	NULL,
};

struct platform_cmd *nyan_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

void gpio_write_port(struct platform_intf *intf, int index, size_t offset, u32 mask, u32 value)
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

void gpio_set_int_enable(struct platform_intf *intf, gpio_t gpio, int enable)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	gpio_write_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
			offsetof(struct gpio_bank, int_enable),
			1 << bit, enable ? (1 << bit) : 0);
}

void gpio_set_out_enable(struct platform_intf *intf, gpio_t gpio, int enable)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	gpio_write_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
			offsetof(struct gpio_bank, out_enable),
			1 << bit, enable ? (1 << bit) : 0);
}

void gpio_set_mode(struct platform_intf *intf, gpio_t gpio, enum gpio_mode mode)
{
	int bit = gpio % GPIO_GPIOS_PER_PORT;
	gpio_write_port(intf, gpio & ((1 << GPIO_PINMUX_SHIFT) - 1),
			offsetof(struct gpio_bank, config),
			1 << bit, mode ? (1 << bit) : 0);
}

void pinmux_set_config(struct platform_intf *intf, int pin_index, uint32_t config)
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

int gpio_get_in_tristate_values(struct platform_intf *intf, gpio_t gpio[], int num_gpio, int value[])
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

int nyan_probe(struct platform_intf *intf)
{
	int index;

	/* nyan-big is listed before google,nyan, so search for it first */
	index = probe_fdt_compatible(&nyan_big_id_list[0],
					ARRAY_SIZE(nyan_big_id_list));
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_big_id_list[index]);
		intf->name = "Big";

        gpio_t gpio[] = {GPIO(Q3), GPIO(T1), GPIO(X1), GPIO(X4)};
        int value[ARRAY_SIZE(gpio)];

            gpio_get_in_tristate_values(intf, gpio, ARRAY_SIZE(gpio), value);

	    if(value[1] == 0 && value[0] == 1)      intf->version_id = "google,nyan-big-rev1";
	    else if(value[1] == 0 && value[0] == 2) intf->version_id = "google,nyan-big-rev2";
	    else if(value[1] == 1 && value[0] == 0) intf->version_id = "google,nyan-big-rev3";
	    else if(value[1] == 1 && value[0] == 1) intf->version_id = "google,nyan-big-rev4";
	    else if(value[1] == 1 && value[0] == 2) intf->version_id = "google,nyan-big-rev5";
	    else if(value[1] == 2 && value[0] == 0) intf->version_id = "google,nyan-big-rev6";
	    else if(value[1] == 2 && value[0] == 1) intf->version_id = "google,nyan-big-rev7";
	    else intf->version_id = "google,nyan-big-rev0";

		return 1;
	}

	/* nyan-blaze is listed before google,nyan, so search for it first */
	index = probe_fdt_compatible(&nyan_blaze_id_list[0],
					ARRAY_SIZE(nyan_blaze_id_list));
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_blaze_id_list[index]);
		intf->name = "Blaze";
		return 1;
	}

	/* nyan-kitty is listed before google,nyan, so search for it first */
	index = probe_fdt_compatible(&nyan_kitty_id_list[0],
					ARRAY_SIZE(nyan_kitty_id_list));
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_kitty_id_list[index]);
		intf->name = "Kitty";
		return 1;
	}

	index = probe_fdt_compatible(&nyan_id_list[0],
					ARRAY_SIZE(nyan_id_list));
	if (index >= 0) {
		lprintf(LOG_DEBUG, "Found platform \"%s\" via FDT compatible "
				"node.\n", nyan_id_list[index]);
		return 1;
	}

	return 0;
}

enum nyan_type get_nyan_type(struct platform_intf *intf)
{
	enum nyan_type ret = NYAN_UNKNOWN;

	if (!strncmp(intf->name, "Big", strlen(intf->name)))
		ret = NYAN_BIG;
	else if (!strncmp(intf->name, "Blaze", strlen(intf->name)))
		ret = NYAN_BLAZE;
	else if (!strncmp(intf->name, "Kitty", strlen(intf->name)))
		ret = NYAN_KITTY;
	else if (!strncmp(intf->name, "Nyan", strlen(intf->name)))
		ret = NYAN;

	return ret;
}

static int nyan_setup_post(struct platform_intf *intf)
{
	if (nyan_ec_setup(intf) <= 0)
		return -1;

	return 0;
}

static int nyan_destroy(struct platform_intf *intf)
{
	intf->cb->ec->destroy(intf);
	return 0;
}

struct eventlog_cb nyan_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
	.add		= &elog_add_event_manually,
	.clear		= &elog_clear_manually,
	.fetch		= &elog_fetch_from_flash,
	.write		= &elog_write_to_flash,
};

struct platform_cb nyan_cb = {
	.ec 		= &cros_ec_cb,
	.eeprom 	= &nyan_eeprom_cb,
	.memory		= &nyan_memory_cb,
	.nvram		= &cros_ec_nvram_cb,
	.sys 		= &nyan_sys_cb,
	.eventlog	= &nyan_eventlog_cb,
};

struct platform_intf platform_nyan = {
	.type		= PLATFORM_ARMV7,
	.name		= "Nyan",
	.id_list	= nyan_id_list,
	.sub		= nyan_sub,
	.cb		= &nyan_cb,
	.probe		= &nyan_probe,
	.setup_post	= &nyan_setup_post,
	.destroy	= &nyan_destroy,
	.version_id   = "google,nyan",
};
