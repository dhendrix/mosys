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
 *
 * gpio.c: GPIO functions for NM10
 */

#include <inttypes.h>

#include "drivers/intel/ich_generic.h"

#include "lib/math.h"

#include "mosys/platform.h"

int nm10_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	return ich_read_gpio(intf, ICH7, gpio);
}

int nm10_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state)
{
	return ich_set_gpio(intf, ICH7, gpio, state);
}

/*
 * nm10_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int nm10_gpio_list(struct platform_intf *intf)
{
	int bank0_gpios[] = { 0,  1,  2,  3,  4,  5,  6,  7,
	                      8,  9,  10, 11, 12, 13, 14, 15,
			      16, 17, 18, 19, 20, 21, 22, 23,
			      24, 25, 26, 27, 28, 29, 30, 31
	};
	int bank1_gpios[] = { 33, 34, 36, 38, 39, 48, 49 };
	int i, rc = 0;

	for (i = 0; i < ARRAY_SIZE(bank1_gpios); i++)
		bank1_gpios[i] = ICH_GPIO_PORT1_TO_PIN(bank1_gpios[i]);

	rc |= ich_gpio_list(intf, ICH7, 0,
	                    &bank0_gpios[0], ARRAY_SIZE(bank0_gpios));
	rc |= ich_gpio_list(intf, ICH7, 1,
	                    &bank1_gpios[0], ARRAY_SIZE(bank1_gpios));

	return rc;
}
