/*
 * Copyright (C) 2011 Google Inc.
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
 */

#ifndef SEABOARD_TEGRA2_H__
#define SEABOARD_TEGRA2_H__

#include <inttypes.h>
#include "mosys/platform.h"

/* platform callbacks */
//extern struct eeprom_cb seaboard_tegra2_eeprom_cb;	/* eeprom.c */
extern struct sys_cb seaboard_tegra2_sys_cb;		/* sys.c */
//extern struct vpd_cb seaboard_tegra2_vpd_cb;		/* vpd.c */
//extern struct gpio_cb seaboard_tegra2_gpio_cb;		/* gpio.c */
//extern struct nvram_cb seaboard_tegra2_nvram_cb;		/* nvram.c */
//extern struct memory_cb seaboard_tegra2_memory_cb;	/* memory.c */

/* functions called by setup routines */
//extern int seaboard_tegra2_vpd_setup(struct platform_intf *intf);
//extern int seaboard_tegra2_eeprom_setup(struct platform_intf *intf);

#endif /* SEABOARD_TEGRA2_H_ */
