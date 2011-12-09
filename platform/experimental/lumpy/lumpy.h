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

#ifndef EXPERIMENTAL_LUMPY_H__
#define EXPERIMENTAL_LUMPY_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define LUMPY_HOST_FIRMWARE_ROM_SIZE		(8192 * 1024)

/* platform callbacks */
extern struct ec_cb lumpy_ec_cb;		/* ec.c */
extern struct eeprom_cb lumpy_eeprom_cb;	/* eeprom.c */
//extern struct gpio_cb lumpy_gpio_cb;		/* gpio.c */
extern struct memory_cb lumpy_memory_cb;	/* memory.c */
extern struct nvram_cb lumpy_nvram_cb;		/* nvram.c */
extern struct sensor_cb lumpy_sensor_cb;	/* sensors.c */
extern struct sysinfo_cb lumpy_sysinfo_cb;	/* sysinfo.c */
extern struct vpd_cb lumpy_vpd_cb;		/* vpd.c */

/* functions called by setup routines */
extern int lumpy_vpd_setup(struct platform_intf *intf);
extern int lumpy_ec_setup(struct platform_intf *intf);
extern int lumpy_ec_destroy(struct platform_intf *intf);
extern int lumpy_eeprom_setup(struct platform_intf *intf);

#endif /* EXPERIMENTAL_LUMPY_H_ */
