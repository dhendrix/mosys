/*
 * Copyright 2012, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
extern int ich_get_gpio_base(struct platform_intf *intf, uint16_t *val);

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

/*
 * ich_global_reset - initiate reset via reset control register (0xcf9)
 *
 * @intf:	platform interface
 *
 * returns <0 to indicate failure
 */
extern int ich_global_reset(struct platform_intf *intf);

#endif /* MOSYS_DRIVERS_INTEL_ICH_H__ */
