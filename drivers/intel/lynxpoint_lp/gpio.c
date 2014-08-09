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
 *
 * gpio.c: GPIO functions for LynxPoint LP
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "drivers/intel/ich_generic.h"
#include "drivers/intel/lpss_generic.h"

#include "lib/math.h"
#include "mosys/kv_pair.h"

int lynxpoint_lp_read_gpio(struct platform_intf *intf, struct gpio_map *gpio)
{
	return lpss_read_gpio(intf, ICH_8_LPSS, gpio);
}

int lynxpoint_lp_set_gpio(struct platform_intf *intf, struct gpio_map *gpio,
			  int state)
{
	return lpss_set_gpio(intf, ICH_8_LPSS, gpio, state);
}

/*
 * lynxpoint_lp_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int lynxpoint_lp_gpio_list(struct platform_intf *intf)
{
	int gpio_ids[94];
	int i;

	for (i = 0; i < ARRAY_SIZE(gpio_ids); i++)
		gpio_ids[i] = i;

	return lpss_gpio_list(intf, ICH_8_LPSS, gpio_ids,
			      ARRAY_SIZE(gpio_ids));
}

/*
 * lynxpoint_lp_list_gpio_attributes - list GPIO's attributes
 *
 * @gpio:	 gpio map
 * @reg: 	 GPIO's attributes
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int lynxpoint_lp_list_gpio_attributes(struct gpio_map *gpio,
				      struct gpio_reg *reg)
{
	return lpss_list_gpio_attributes(gpio, reg);
}

/*
 * lynxpoint_lp_read_gpio_attributes - list GPIO's attributes
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 * @reg: 	GPIO's attributes
 *
 * returns 0 if successful
 * returns <0 if failure
 */
int lynxpoint_lp_read_gpio_attributes(struct platform_intf *intf,
				      struct gpio_map *gpio,
				      struct gpio_reg *reg)
{
	return lpss_read_gpio_attributes(intf, ICH_8_LPSS, gpio, reg);
}
