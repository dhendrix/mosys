/*
 * Copyright 2013, Google Inc.
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

#ifndef MOSYS_DRIVERS_INTEL_LPSS_GENERIC_H__
#define MOSYS_DRIVERS_INTEL_LPSS_GENERIC_H__

#define LPSS_GPIO_OWN(num)		((num / 32) * 4)
#define LPSS_GPIO_ROUT(num)		(0x30 + (num / 32) * 4)
#define LPSS_GPIO_IE(num)		(0x90 + (num / 32) * 4)
#define LPSS_GPIO_CONF0(num)		(0x100 + ((num) * 8))
#define  LPSS_GPIO_CONF0_MODE_BIT	0
#define  LPSS_GPIO_CONF0_DIR_BIT	2
#define  LPSS_GPIO_CONF0_INV_BIT	3
#define  LPSS_GPIO_CONF0_GPI_BIT	30
#define  LPSS_GPIO_CONF0_GPO_BIT	31

/* forward declarations */
struct platform_intf;
struct gpio_map;
enum ich_generation;

/* Boot BIOS Straps for LPSS chipsets */
enum ich_lpss_bbs {
	LPSS_BBS_UNKNOWN	= -1,
	LPSS_BBS_SPI		= 0,
	LPSS_BBS_LPC		= 1,
};

struct gpio_reg {
	uint8_t type;
	uint8_t state;
	uint8_t pirq;
	uint8_t ownership;
	uint8_t smi_en;
	uint8_t rout;
	uint8_t interrupt_en;
};

/*
 * lpss_get_bbs - get bios boot straps (bbs) value
 *
 * @intf:	platform interface
 *
 * returns BBS value to indicate success
 * returns <0 to indicate failure
 */
int lpss_get_bbs(struct platform_intf *intf);

/*
 * lpss_set_bbs - set bios boot straps (bbs) value
 *
 * @intf:	platform interface
 * @bbs:	bbs value
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int lpss_set_bbs(struct platform_intf *intf, int bbs);

/*
 * lpss_get_gpio_base  - get GPIO base address.
 *
 * @intf:	platform interface
 * @val:	location to store value
 *
 * This function is primarily useful when setting values that are related to
 * GPIOs. For reading and writing GPIOs themselves, use the lpss_read_gpio and
 * lpss_set_gpio wrapper functions for added safety.
 *
 * returns 0 to indicate success
 * returns <0 on read failure
 */
int lpss_get_gpio_base(struct platform_intf *intf, uint32_t *val);

/*
 * lpss_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
int lpss_read_gpio(struct platform_intf *intf,
		   enum ich_generation gen, struct gpio_map *gpio);

/*
 * lpss_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int lpss_set_gpio(struct platform_intf *intf, enum ich_generation gen,
		  struct gpio_map *gpio, int state);

/*
 * lpss_gpio_list  -  list GPIOs
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio_ids:	set of GPIO IDs to list
 * @num_gpios:	number of GPIOs in set
 *
 * Note: Some chipsets do not implement certain GPIOs. Use the gpios
 * array to specify which GPIOs should be listed by default.
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int lpss_gpio_list(struct platform_intf *intf, enum ich_generation gen,
		   int gpio_ids[], int num_gpios);
/*
 * llpss_list_gpio_attributes -  list GPIOs attributes
 *
 * @gpio:	gpio map
 * @reg: 	contains GPIO's attributes
 *
 * Note: Some chipsets do not implement certain GPIOs. Use the gpios
 * array to specify which GPIOs should be listed by default.
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int lpss_list_gpio_attributes(struct gpio_map *gpio, struct gpio_reg *reg);

/*
 * lpss_read_gpio_attributes  -  list GPIOs
 *
 * @intf:	platform interface
 * @gen: 	chipset generation
 * @gpio:	gpio map
 * @reg: 	contains GPIO's attributes
 *
 * Note: Some chipsets do not implement certain GPIOs. Use the gpios
 * array to specify which GPIOs should be listed by default.
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int lpss_read_gpio_attributes(struct platform_intf *intf,
			      enum ich_generation gen, struct gpio_map *gpio,
			      struct gpio_reg *reg);

#endif /* MOSYS_DRIVERS_INTEL_LPSS_GENERIC_H__ */
