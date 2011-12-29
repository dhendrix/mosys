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

#ifndef MOSYS_DRIVERS_INTEL_NM10_H__
#define MOSYS_DRIVERS_INTEL_NM10_H__

#include "drivers/gpio.h"
#include "drivers/intel/ich_generic.h"

/*
 * The NM10 has one SMBus interface, so for the purpose of matching with sysfs
 * entries we don't need to worry about the exact IO address which appears at
 * the end of the string (e.g. "SMBus I801 adapter at 2000").
 */
#define NM10_SMBUS_ADAPTER	"SMBus I801 adapter"

/*
  * nm10_get_bbs - get bios boot straps (bbs) value
  *
  * @intf:	platform interface
  *
  * returns BBS value to indicate success
  * returns <0 to indicate failure
  */
enum ich_bbs_ich7 nm10_get_bbs(struct platform_intf *intf);

/*
  * nm10_set_bbs - set bios boot straps (bbs) value
  *
  * @intf:	platform interface
  * @bbs:	bbs value
  *
  * returns 0 to indicate success
  * returns <0 to indicate failure
  */
int nm10_set_bbs(struct platform_intf *intf, enum ich_bbs_ich7 bbs);

/*
 * nm10_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
int nm10_read_gpio(struct platform_intf *intf, struct gpio_map *gpio);

/*
 * nm10_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int nm10_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state);

/*
 * nm10_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
extern int nm10_gpio_list(struct platform_intf *intf);

#endif /* MOSYS_DRIVERS_INTEL_NM10_H__ */
