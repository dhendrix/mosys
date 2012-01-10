/*
 * Copyright (C) 2010 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * series6.c: Intel 6-series chipset helper functions. For now, this is
 * trivial wrappers around similar NM10 helper functions.
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/ich_generic.h"
#include "drivers/intel/series6.h"

#include "intf/io.h"

enum ich_snb_bbs series6_get_bbs(struct platform_intf *intf)
{
	enum ich_snb_bbs val;

	if ((val = ich_get_bbs(intf)) < 0)
		return ICH_SNB_BBS_UNKNOWN;

	return val;
}

int series6_set_bbs(struct platform_intf *intf, enum ich_snb_bbs bbs)
{
	if (bbs == ICH_SNB_BBS_UNKNOWN)
		return -1;

	return ich_set_bbs(intf, bbs);
}

static int series6_send_suswarn(struct platform_intf *intf)
{
	struct gpio_map gpio30 = {
		.type	= GPIO_OUT,
		.port	= 0,
		.pin	= 30,
	};
	uint16_t gpio_base;
	uint32_t val;

	if (ich_get_gpio_base(intf, &gpio_base) < 0)
		return -1;

	/* set state of GPIO30 as GPIO output and drive low */
	series6_set_gpio(intf, &gpio30, 0);

	/* set GPIO30 to reset by RSMRST only */
	if (io_read32(intf, gpio_base, &val) < 0)
		return -1;;
	if (io_write32(intf, gpio_base + 0x60, val | (1 << 30)) < 0)
		return -1;

	return 0;
}

int series6_global_reset(struct platform_intf *intf)
{
	/* Send SUSWARN#, especially if platform has an EC */
	if (series6_send_suswarn(intf))
		return -1;

	/* if it returns, something went wrong */
	return ich_global_reset(intf);
}
