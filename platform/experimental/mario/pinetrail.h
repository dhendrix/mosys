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
 */

#ifndef MARIO_PINETRAIL_H__
#define MARIO_PINETRAIL_H__

#include <inttypes.h>
#include "mosys/platform.h"

/* platform callbacks */
extern struct eeprom_cb mario_pinetrail_eeprom_cb;	/* eeprom.c */
extern struct gpio_cb mario_pinetrail_gpio_cb;		/* gpio.c */
extern struct memory_cb mario_pinetrail_memory_cb;	/* memory.c */
extern struct sysinfo_cb mario_pinetrail_sysinfo_cb;	/* sysinfo.c */
extern struct vpd_cb mario_pinetrail_vpd_cb;		/* vpd.c */
extern struct nvram_cb mario_pinetrail_nvram_cb;	/* nvram.c */
extern struct ec_cb mario_pinetrail_ec_cb;		/* ec.c */

/* functions called by setup routines */
extern int mario_pinetrail_vpd_setup(struct platform_intf *intf);
extern int mario_pinetrail_eeprom_setup(struct platform_intf *intf);
extern int mario_pinetrail_ec_setup(struct platform_intf *intf);

/* misc */
extern char *mario_pinetrail_ec_mbid(struct platform_intf *intf);

#endif /* MARIO_PINETRAIL_H_ */
