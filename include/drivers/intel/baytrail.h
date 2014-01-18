/*
 * Copyright 2014, Google Inc.
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

#ifndef MOSYS_DRIVERS_INTEL_BAYTRAIL_H__
#define MOSYS_DRIVERS_INTEL_BAYTRAIL_H__

/* forward declarations */
struct platform_intf;
struct gpio_map;

/* Baytrail has three different GPIO banks / ports. */
enum baytrail_gpio_port
{
	BAYTRAIL_GPNCORE_PORT,
	BAYTRAIL_GPSCORE_PORT,
	BAYTRAIL_GPSSUS_PORT,
	BAYTRAIL_NUM_PORTS,
};

/* Number of GPIOs in each bank / port. */
enum baytrail_gpio_count
{
	BAYTRAIL_GPNCORE_COUNT		= 27,
	BAYTRAIL_GPSCORE_COUNT		= 102,
	BAYTRAIL_GPSSUS_COUNT		= 44,
};

/* Address of each bank / port, (IO_BASE addr) */
enum baytrail_io_base_offset
{
	BAYTRAIL_GPNCORE_OFFSET		= 0x1000,
	BAYTRAIL_GPSCORE_OFFSET		= 0x0000,
	BAYTRAIL_GPSSUS_OFFSET		= 0x2000,
};

/* Bank description */
struct baytrail_gpio_bank {
	const int gpio_count;
	const uint8_t *gpio_to_pad;
	const uint32_t io_base_offset;
};

/* Each GPIO pin has 2 configuration registers and 1 value register. These
 * macros build the addresses for these registers. */
#define BAYTRAIL_GPIO_CONF0(base,bank,gpio)	\
	((base) + (bank)->gpio_to_pad[(gpio)] * 16 + (bank)->io_base_offset + 0)
#define BAYTRAIL_GPIO_CONF1(base,bank,gpio)	\
	((base) + (bank)->gpio_to_pad[(gpio)] * 16 + (bank)->io_base_offset + 4)
#define BAYTRAIL_GPIO_VAL(base,bank,gpio)	\
	((base) + (bank)->gpio_to_pad[(gpio)] * 16 + (bank)->io_base_offset + 8)

/* VAL register bits */
#define BAYTRAIL_GPIO_VAL_GPIO_BIT		0
#define BAYTRAIL_GPIO_VAL_OUTPUT_DISABLE_BIT	1
#define BAYTRAIL_GPIO_VAL_INPUT_DISABLE_BIT	2

/*
 * baytrail_read_gpio	- read GPIO status
 * @intf:		platform interface
 * @gpio:		gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */
int baytrail_read_gpio(struct platform_intf *intf, struct gpio_map *gpio);


/*
 * baytrail_set_gpio 	- set GPIO status
 * @intf:	platform interface
 * @gpio	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int baytrail_set_gpio(struct platform_intf *intf, struct gpio_map *gpio,
		      int state);


#endif /* MOSYS_DRIVERS_INTEL_BAYTRAIL_H__ */
