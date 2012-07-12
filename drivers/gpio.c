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
 * gpio.c: GPIO functions generic to all platforms
 */

#include "mosys/kv_pair.h"
#include "mosys/log.h"

#include "drivers/gpio.h"

/*
 * kv_pair_print_gpio  -  print gpio info and state
 *
 * @gpio:	gpio data
 * @state:	gpio state
 */
void kv_pair_print_gpio(struct gpio_map *gpio, int state)
{
	struct kv_pair *kv;

	kv = kv_pair_new();
	kv_pair_add(kv, "device", gpio->devname);
	kv_pair_fmt(kv, "id", "GPIO%02u", gpio->id);

	switch (gpio->type) {
	case GPIO_IN:
		kv_pair_add(kv, "type", "IN");
		break;
	case GPIO_OUT:
		kv_pair_add(kv, "type", "OUT");
		break;
	case GPIO_ALT:
		kv_pair_add(kv, "type", "ALT");
		break;
	default:
		lprintf(LOG_DEBUG, "Invalid GPIO type %d\n", gpio->type);
		kv_pair_free(kv);
		return;
	}

	kv_pair_fmt(kv, "state", "%d", state);
	kv_pair_add(kv, "name", gpio->name);

	kv_pair_print(kv);
	kv_pair_free(kv);
}
