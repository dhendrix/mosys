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

#ifndef ACER_CHROMIA700_H__
#define ACER_CHROMIA700_H__

#include <inttypes.h>
#include "mosys/platform.h"

/* platform callbacks */
extern struct ec_cb acer_chromia700_ec_cb;		/* ec.c */
extern struct eeprom_cb acer_chromia700_eeprom_cb;	/* eeprom.c */
extern struct sys_cb acer_chromia700_sys_cb;		/* sys.c */
extern struct vpd_cb acer_chromia700_vpd_cb;		/* vpd.c */
extern struct gpio_cb acer_chromia700_gpio_cb;		/* gpio.c */
extern struct nvram_cb acer_chromia700_nvram_cb;	/* nvram.c */
extern struct memory_cb acer_chromia700_memory_cb;	/* memory.c */

/* functions called by setup routines */
extern int acer_chromia700_ec_setup(struct platform_intf *intf);
extern void acer_chromia700_ec_destroy(struct platform_intf *intf);
extern int acer_chromia700_vpd_setup(struct platform_intf *intf);
extern int acer_chromia700_eeprom_setup(struct platform_intf *intf);

#endif /* ACER_CHROMIA700_H_ */
