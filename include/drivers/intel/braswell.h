/*
 * Copyright 2015, Google Inc.
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

#ifndef MOSYS_DRIVERS_INTEL_BRASWELL_H__
#define MOSYS_DRIVERS_INTEL_BRASWELL_H__

/* forward declarations */
struct platform_intf;
struct gpio_map;

/* Braswell has four different GPIO banks / ports. */
enum braswell_gpio_port {
	BSW_GPNCORE_PORT,
	BSW_GPSECORE_PORT,
	BSW_GPSWCORE_PORT,
	BSW_GPECORE_PORT,
	BRASWELL_NUM_PORTS,
};

/* Number of GPIOs in each bank / port. */
enum braswell_gpio_count {
	BSW_GPNCORE_COUNT	= 59,
	BSW_GPSECORE_COUNT	= 55,
	BSW_GPSWCORE_COUNT	= 56,
	BSW_GPECORE_COUNT	= 24,
};

/* Address of each bank / port, (IO_BASE addr) */
enum braswell_community_base_offset {
	BSW_GPNCORE_OFFSET	= 0x08000,
	BSW_GPSWCORE_OFFSET	= 0x00000,
	BSW_GPSECORE_OFFSET	= 0x18000,
	BSW_GPECORE_OFFSET	= 0x10000,

};

/* Bank description */
struct braswell_gpio_bank {
	const int gpio_count;
	const uint8_t *gpio_to_pad;
	const uint32_t community_base_offset;
};

/* Macros to calculate the offset for the GPIO */

#define MAX_FAMILY_PAD_GPIO_NO	15
#define BIT(nr)			(1UL << (nr))
#define BSW_GPIO_RX_STAT	BIT(0)
#define BSW_GPIO_TX_STAT	BIT(1)
#define BSW_GPIO_CFG_MASK	(BIT(8) | BIT(9) | BIT(10))
#define BSW_GPIO_IN_OUT		0
#define BSW_GPIO_OUT		1
#define BSW_GPIO_IN		2
#define BSW_GPIO_HIZ		3
#define BRASWELL_FAMILY_NUMBER(gpio) \
	(gpio->id / MAX_FAMILY_PAD_GPIO_NO)

#define BRASWELL_PAD_NUMBER(gpio) \
	(gpio->id % MAX_FAMILY_PAD_GPIO_NO)

#define BSW_GPIO_CONF0(io_base, bank, gpio) \
	((io_base) + bank->community_base_offset + 0x4400 \
	+ 0x400 * BRASWELL_FAMILY_NUMBER(gpio) + 8 * BRASWELL_PAD_NUMBER(gpio))

/*
 * braswell_read_gpio	- read GPIO status
 * @intf:		platform interface
 * @gpio:		gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
int braswell_read_gpio(struct platform_intf *intf, struct gpio_map *gpio);


/*
 * braswell_set_gpio	- set GPIO status
 * @intf:	platform interface
 * @gpio	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int braswell_set_gpio(struct platform_intf *intf, struct gpio_map *gpio,
		      int state);


#endif /* MOSYS_DRIVERS_INTEL_BRASWELL_H__ */
