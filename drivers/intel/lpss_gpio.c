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
 * gpio.c: GPIO functions for Intel LPSS chipsets
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "intf/io.h"
#include "intf/pci.h"

#include "drivers/gpio.h"
#include "drivers/intel/ich_generic.h"
#include "drivers/intel/lpss_generic.h"
#include "mosys/kv_pair.h"

int lpss_get_gpio_base(struct platform_intf *intf, uint32_t *val)
{
	static uint32_t lpss_gpio_base;

	if (lpss_gpio_base) {
		*val = lpss_gpio_base;
		return 0;
	}

	if (pci_read32(intf, 0, 31, 0, 0x48, &lpss_gpio_base) < 0) {
		lprintf(LOG_DEBUG, "LPSS GPIO: unable to find base\n");
		return -1;
	}

	/* lsb is enable bit */
	lpss_gpio_base &= ~1;

	lprintf(LOG_DEBUG, "LPSS GPIO: base is 0x%08x\n", lpss_gpio_base);
	*val = lpss_gpio_base;
	return 0;
}

/* returns 1 if GPIO is valid, 0 otherwise */
static int lpss_gpio_valid(enum ich_generation gen, struct gpio_map *gpio)
{
	switch (gen) {
	case ICH_8_LPSS:
		/* Series 8 LPSS supports 94 GPIOs */
		if (gpio->id > 94) {
			lprintf(LOG_ERR, "LPSS GPIO: invalid ID %d\n",
				gpio->id);
			return 0;
		}
		break;
	default:
		lprintf(LOG_ERR, "LPSS GPIO: invalid ICH gen %d\n", gen);
		return 0;
	}

	return 1;
}

static int check_pirq_gpio_valid(int gpio)
{
	switch (gpio) {
		case 8:  return 0;	/* PIRQI */
		case 9:  return 1;	/* PIRQJ */
		case 10: return 2;	/* PIRQK */
		case 13: return 3;	/* PIRQL */
		case 14: return 4;	/* PIRQM */
		case 45: return 5;	/* PIRQN */
		case 46: return 6;	/* PIRQO */
		case 47: return 7;	/* PIRQP */
		case 48: return 8;	/* PIRQQ */
		case 49: return 9;	/* PIRQR */
		case 50: return 10;	/* PIRQS */
		case 51: return 11;	/* PIRQT */
		case 52: return 12;	/* PIRQU */
		case 53: return 13;	/* PIRQV */
		case 54: return 14;	/* PIRQW */
		case 55: return 15;	/* PIRQX */
		default: return -1;
	};
}

int lpss_list_gpio_attributes(struct gpio_map *gpio, struct gpio_reg *reg)
{
	struct kv_pair *kv;
	int rc;

	kv = kv_pair_new();
	kv_pair_add(kv, "device", gpio->devname);
	kv_pair_fmt(kv, "id", "GPIO%02u", gpio->id);

	/* The following strings will be alligned */
	switch (gpio->type) {
		case GPIO_IN:
			kv_pair_add(kv, "type", "IN    ");
			break;
		case GPIO_OUT:
			kv_pair_add(kv, "type", "OUT   ");
			break;
		case GPIO_ALT:
			kv_pair_add(kv, "type", "NATIVE");
			break;
		default:
			lprintf(LOG_DEBUG, "Invalid GPIO type %d\n",
				gpio->type);
			kv_pair_free(kv);
	}

	kv_pair_fmt(kv, "state", "%d", reg->state);

	if (reg->ownership)
		kv_pair_add(kv, "owner", "GPIO");
	else
		kv_pair_add(kv, "owner", "ACPI");

	if (reg->rout)
		kv_pair_add(kv, "owner", "SMI");
	else
		kv_pair_add(kv, "owner", "SCI");

	if (gpio->neg)
		kv_pair_add(kv, "neg", "NEG ON ");
	else
		kv_pair_add(kv, "neg", "NEG OFF");

	if (reg->pirq == 1)
		kv_pair_add(kv, "pirq", "PIRQ ON ");
	else
		kv_pair_add(kv, "pirq", "PIRQ OFF");

	rc = kv_pair_print(kv);
	kv_pair_free(kv);
	return rc;
}


int lpss_read_gpio_attributes(struct platform_intf *intf,
			      enum ich_generation gen, struct gpio_map *gpio,
			      struct gpio_reg *reg)
{
	uint32_t base, val;
	int pirq;

	if (!lpss_gpio_valid(gen, gpio))
		return -1;
	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;

	if (io_read32(intf, base + LPSS_GPIO_CONF0(gpio->id), &val) < 0)
		return -1;

	if (val & (1 << LPSS_GPIO_CONF0_DIR_BIT)) {
		gpio->type = GPIO_IN;
		reg->state = (val >> LPSS_GPIO_CONF0_GPI_BIT) & 1;
	} else {
		gpio->type = GPIO_OUT;
		reg->state = (val >> LPSS_GPIO_CONF0_GPO_BIT) & 1;
	}

	if (!(val & (1 << LPSS_GPIO_CONF0_MODE_BIT)))
		gpio->type = GPIO_ALT;

	gpio->neg = (val >> LPSS_GPIO_CONF0_INV_BIT) & 1;

	if (io_read32(intf, base + LPSS_GPIO_OWN(gpio->id), &val) < 0)
		return -1;
	reg->ownership = ((val >> (gpio->id % 32)) & 1);

	if (io_read32(intf, base + LPSS_GPIO_IE(gpio->id), &val) < 0)
		return -1;
	reg->interrupt_en = ((val >> (gpio->id % 32)) & 1);

	if (io_read32(intf, base + LPSS_GPIO_ROUT(gpio->id), &val) < 0)
		return -1;
	reg->rout = ((val >> (gpio->id % 32)) & 1);

	pirq = check_pirq_gpio_valid(gpio->id);
	reg->pirq = 0xff;
	if ( pirq > -1) {
		if (io_read32(intf, base + 0x10, &val) < 0)
			return -1;
		val &= 0x0ffff;
		reg->pirq = (val >> pirq) & 1;
	}

	return 0;
}

int lpss_read_gpio(struct platform_intf *intf,
		   enum ich_generation gen, struct gpio_map *gpio)
{
	uint32_t base, val;

	if (!lpss_gpio_valid(gen, gpio))
		return -1;
	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;
	if (io_read32(intf, base + LPSS_GPIO_CONF0(gpio->id), &val) < 0)
		return -1;

	return ((val >> LPSS_GPIO_CONF0_GPI_BIT) & 1);
}

int lpss_set_gpio(struct platform_intf *intf, enum ich_generation gen,
		  struct gpio_map *gpio, int state)
{
	uint32_t base, addr, val;

	if (gpio->type != GPIO_OUT) {
		lprintf(LOG_DEBUG, "Must set \"%s\" to output mode to change "
		        "its value\n", gpio->name ? gpio->name : "Unknown");
		return -1;
	}

	if (!lpss_gpio_valid(gen, gpio))
		return -1;
	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;

	addr = base + LPSS_GPIO_CONF0(gpio->id);
	if (io_read32(intf, addr, &val) < 0)
		return -1;

	/* Check if the GPIO is native mode */
	if (!(val & (1 << LPSS_GPIO_CONF0_MODE_BIT))) {
		lprintf(LOG_DEBUG, "Changing GPIO ID %d from native to "
			"GPIO mode\n", gpio->id);

		val |= (1 << LPSS_GPIO_CONF0_MODE_BIT);
		if (io_write32(intf, addr, val) < 0)
			return -1;
	}

	/* Check if the GPIO is output */
	if (val & (1 << LPSS_GPIO_CONF0_DIR_BIT)) {
		lprintf(LOG_DEBUG, "Changing GPIO ID %d from input to "
		        "output\n", gpio->id);

		val &= ~(1 << LPSS_GPIO_CONF0_DIR_BIT);
		if (io_write32(intf, addr, val) < 0)
			return -1;
	}

	/* Set the output value */
	switch (state) {
	case 0:
		if (!(val & (1 << LPSS_GPIO_CONF0_GPO_BIT))) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 0\n", gpio->name);
		} else {
			val &= ~(1 << LPSS_GPIO_CONF0_GPO_BIT);
			if (io_write32(intf, addr, val) < 0)
				return -1;
		}
		break;
	case 1:
		if (val & (1 << LPSS_GPIO_CONF0_GPO_BIT)) {
			lprintf(LOG_DEBUG, "GPIO '%s' already 1\n", gpio->name);
		} else {
			val |= (1 << LPSS_GPIO_CONF0_GPO_BIT);
			if (io_write32(intf, addr, val) < 0)
				return -1;
		}
		break;
	default:
		lprintf(LOG_ERR, "Invaild state %d\n", state);
		return -1;
	}

	return 0;
}

int lpss_gpio_list(struct platform_intf *intf, enum ich_generation gen,
		   int gpio_ids[], int num_gpios)
{
	int i, state;
	uint32_t base, addr, val;

	if (lpss_get_gpio_base(intf, &base) < 0)
		return -1;

	for (i = 0; i < num_gpios; i++) {
		struct gpio_map gpio;
		char tmp[16];

		memset(&gpio, 0, sizeof(gpio));

		gpio.id = gpio_ids[i];
		gpio.port = gpio.id % 32;
		gpio.pin = gpio.id / 32;

		if (!lpss_gpio_valid(gen, &gpio))
			return -1;

		addr = base + LPSS_GPIO_CONF0(gpio.id);
		if (io_read32(intf, addr, &val) < 0)
			return -1;

		/* skip if signal is "native function" instead of GPIO */
		if (!(val & (1 << LPSS_GPIO_CONF0_MODE_BIT)))
			continue;

		if (val & (1 << LPSS_GPIO_CONF0_DIR_BIT))
			gpio.type = GPIO_IN;
		else
			gpio.type = GPIO_OUT;

		if (val & (1 << LPSS_GPIO_CONF0_INV_BIT))
			gpio.neg = 1;

		sprintf(tmp, "LPSS");
		gpio.devname = mosys_strdup(tmp);

		sprintf(tmp, "GPIO%02d", gpio.id);
		gpio.name = mosys_strdup(tmp);

		state = lpss_read_gpio(intf, gen, &gpio);
		if (state >= 0)
			kv_pair_print_gpio(&gpio, state);

		free((void *)gpio.devname);
		free((void *)gpio.name);
	}

	return 0;
}
