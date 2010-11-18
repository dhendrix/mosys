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

#ifndef ALEX_PINETRAIL_H__
#define ALEX_PINETRAIL_H__

#include <inttypes.h>
#include "mosys/platform.h"

/* platform callbacks */
extern struct eeprom_cb alex_pinetrail_eeprom_cb;	/* eeprom.c */
extern struct gpio_cb alex_pinetrail_gpio_cb;		/* gpio.c */
extern struct sysinfo_cb alex_pinetrail_sysinfo_cb;	/* sysinfo.c */
extern struct vpd_cb alex_pinetrail_vpd_cb;		/* vpd.c */
extern struct nvram_cb alex_pinetrail_nvram_cb;		/* nvram.c */

/* functions called by setup routines */
extern int alex_pinetrail_vpd_setup(struct platform_intf *intf);
extern int alex_pinetrail_eeprom_setup(struct platform_intf *intf);

#endif /* ALEX_PINETRAIL_H_ */
