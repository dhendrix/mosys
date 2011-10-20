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

#ifndef EXPERIMENTAL_STUMPY_H__
#define EXPERIMENTAL_STUMPY_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define STUMPY_HOST_FIRMWARE_ROM_SIZE		(8192 * 1024)

/* platform callbacks */
extern struct eeprom_cb stumpy_eeprom_cb;	/* eeprom.c */
//extern struct gpio_cb stumpy_gpio_cb;		/* gpio.c */
extern struct memory_cb stumpy_memory_cb;	/* memory.c */
extern struct nvram_cb stumpy_nvram_cb;		/* nvram.c */
extern struct sensor_cb stumpy_sensor_cb;	/* sensors.c */
extern struct sysinfo_cb stumpy_sysinfo_cb;	/* sysinfo.c */
extern struct vpd_cb stumpy_vpd_cb;		/* vpd.c */

/* functions called by setup routines */
extern int stumpy_superio_setup(struct platform_intf *intf);
extern void stumpy_superio_destroy(struct platform_intf *intf);
extern int stumpy_vpd_setup(struct platform_intf *intf);
extern int stumpy_eeprom_setup(struct platform_intf *intf);

#endif /* EXPERIMENTAL_STUMPY_H_ */
