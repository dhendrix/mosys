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

#ifndef MOSYS_DRIVERS_SAMSUNG_EXYNOS_GENERIC_H__
#define MOSYS_DRIVERS_SAMSUNG_EXYNOS_GENERIC_H__

/* forward declarations */
struct platform_intf;
struct gpio_map;

enum exynos_generation {
	EXYNOS4,
	EXYNOS5,
};

struct exynos_gpio_bank {
	const char *name;	/* name of port (GPAn, GPBn, etc) */
	uint32_t baseaddr;	/* base address */
};

/* Except for ETCn, each bank has 6 32-bit registers */
struct exynos_gpio_regs {
	uint32_t con;		/* config */
	uint32_t dat;		/* data */
	uint32_t pud;		/* pull up/down */
	uint32_t drv_sr;	/* drive strength */
	uint32_t conpdn;	/* power down mode config */
	uint32_t pudpdn;	/* power down mode pull up/down */
} __attribute__ ((packed));

/*
 * exynos_get_gpio_regs  - fill in GPIO register info for a given GPIO
 *
 * @gpio:	high-level GPIO information
 * @regs:	GPIO register set
 *
 * This function is primarily useful when obtaining information GPIO
 * configuration and MMIO addresses.
 *
 * returns 0 to indicate success
 * returns <0 on read failure
 */
extern int exynos_get_gpio_regs(struct gpio_map *gpio,
				struct exynos_gpio_regs *regs);

/*
 * exynos_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
extern int exynos_read_gpio(struct platform_intf *intf,
			    enum exynos_generation gen, struct gpio_map *gpio);

/*
 * exynos_read_gpio_mvl  - read many-value logic GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 *
 * returns GPIO state as 0, 1, or Z
 * returns <0 on read failure
 */
extern int exynos_read_gpio_mvl(struct platform_intf *intf,
				enum exynos_generation gen,
				struct gpio_map *gpio);

/*
 * exynos_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gen:	chipset generation
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
extern int exynos_set_gpio(struct platform_intf *intf,
			   enum exynos_generation gen,
			   struct gpio_map *gpio, int state);

/*
 * exynos_gpio_list  -  list GPIOs in a given bank
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
extern int exynos_gpio_list(struct platform_intf *intf,
			    enum exynos_generation gen);

#endif /* MOSYS_DRIVERS_SAMSUNG_EXYNOS_H__ */
