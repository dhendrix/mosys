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

#ifndef MOSYS_DRIVERS_INTEL_SERIES6_H__
#define MOSYS_DRIVERS_INTEL_SERIES6_H__

struct platform_intf;
struct gpio_map;

/*
 * The 6-Series has one SMBus interface, so for the purpose of matching with
 * sysfs entries we don't need to worry about the exact IO address which appears
 * at the end of the string (e.g. "SMBus I801 adapter at 2000").
 */
#define SERIES6_SMBUS_ADAPTER	"SMBus I801 adapter"

/*
 * series6_get_bbs - get bios boot straps (bbs) value
 *
 * @intf:	platform interface
 *
 * returns BBS value to indicate success
 * returns <0 to indicate failure
 */
enum ich_snb_bbs series6_get_bbs(struct platform_intf *intf);

/*
  * series6_set_bbs - set bios boot straps (bbs) value
  *
  * @intf:	platform interface
  * @bbs:	bbs value
  *
  * returns 0 to indicate success
  * returns <0 to indicate failure
  */
int series6_set_bbs(struct platform_intf *intf, enum ich_snb_bbs bbs);

/*
 * series6_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */

int series6_read_gpio(struct platform_intf *intf, struct gpio_map *gpio);

/*
 * series6_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int series6_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state);

/*
 * series6_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
extern int series6_gpio_list(struct platform_intf *intf);

/*
 * series6_global_reset - force global reset
 *
 * @intf:	platform interface
 *
 * Obviously this function should force the platform to die before
 * returning, but we have a return code anyway...
 *
 * returns 0 if successful
 * returns <0 if failure
 */
extern int series6_global_reset(struct platform_intf *intf);

#endif /* MOSYS_DRIVERS_INTEL_SERIES6_H__ */
