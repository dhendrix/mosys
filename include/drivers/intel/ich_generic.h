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

#ifndef MOSYS_DRIVERS_INTEL_ICH_H__
#define MOSYS_DRIVERS_INTEL_ICH_H__

/* convert GPIO# to pin# */
#define ICH_GPIO_PORT1_TO_PIN(x)	(32 - (x))
#define ICH_GPIO_PORT2_TO_PIN(x)	(64 - (x))

/* forward declarations */
struct platform_intf;
struct gpio_map;

enum ich_generation {
	ICH7,		/* ICH7 and NM10 */
	ICH8,
	ICH9,
	ICH10,
	ICH_6_SERIES,	/* Cougar Point */
	ICH_7_SERIES,	/* Panther Point */
};

/* Boot BIOS Straps for ICH7-ICH10 chipsets */
enum ich_bbs_ich7 {
	ICH7_BBS_UNKNOWN	= -1,
	ICH7_BBS_RSVD		= 0x0,	/* note: this is also SPI on ICH10 */
	ICH7_BBS_SPI		= 0x1,
	ICH7_BBS_PCI		= 0x2,
	ICH7_BBS_LPC		= 0x3,
};

/* Boot BIOS Straps for Sandy Bridge and newer chipsets */
enum ich_snb_bbs {
	ICH_SNB_BBS_UNKNOWN	= -1,
	ICH_SNB_BBS_LPC		= 0x0,
	ICH_SNB_BBS_RSVD	= 0x1,
	ICH_SNB_BBS_PCI		= 0x2,
	ICH_SNB_BBS_SPI		= 0x3,
};

/*
 * ich_get_bbs - get bios boot straps (bbs) value
 *
 * @intf:	platform interface
 *
 * returns BBS value to indicate success
 * returns <0 to indicate failure
 */
extern int ich_get_bbs(struct platform_intf *intf);

/*
 * ich_set_bbs - set bios boot straps (bbs) value
 *
 * @intf:	platform interface
 * @bbs:	bbs value
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int ich_set_bbs(struct platform_intf *intf, int bbs);


/*
 * ich_get_gpio_base  - get GPIO base address.
 *
 * @intf:	platform interface
 * @val:	location to store value
 *
 * This function is primarily useful when setting values that are related to
 * GPIOs. For reading and writing GPIOs themselves, use the ich_read_gpio and
 * ich_set_gpio wrapper functions for added safety.
 * 
 * returns 0 to indicate success
 * returns <0 on read failure
 */
extern int ich_get_gpio_base(struct platform_intf *intf, uint32_t *val);

/*
 * ich_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
int ich_read_gpio(struct platform_intf *intf,
                  enum ich_generation gen, struct gpio_map *gpio);

/*
 * ich_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int ich_set_gpio(struct platform_intf *intf, enum ich_generation gen,
                 struct gpio_map *gpio, int state);

/*
 * ich_gpio_list  -  list GPIOs in a given bank
 *
 * @intf:	platform interface
 * @port:	GPIO port
 * @gpio_pins:	set of GPIO pins to list
 * @num_gpios:	number of GPIOs in set
 *
 * Note: Some chipsets do not implement certain GPIOs. Use the gpios
 * array to specify which GPIOs should be listed by default.
 *
 * returns 0 if successful
 * returns <0 if failure
 */
extern int ich_gpio_list(struct platform_intf *intf, enum ich_generation gen,
                         int port, int gpio_pins[], int num_gpios);

#endif /* MOSYS_DRIVERS_INTEL_ICH_H__ */
