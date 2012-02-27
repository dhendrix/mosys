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
 * series6.c: Intel 6-series chipset helper functions. For now, this is
 * trivial wrappers around similar NM10 helper functions.
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "drivers/gpio.h"
#include "drivers/intel/ich_generic.h"
#include "drivers/intel/series6.h"

#include "intf/io.h"

enum ich_snb_bbs series6_get_bbs(struct platform_intf *intf)
{
	enum ich_snb_bbs val;

	if ((val = ich_get_bbs(intf)) < 0)
		return ICH_SNB_BBS_UNKNOWN;

	return val;
}

int series6_set_bbs(struct platform_intf *intf, enum ich_snb_bbs bbs)
{
	if (bbs == ICH_SNB_BBS_UNKNOWN)
		return -1;

	return ich_set_bbs(intf, bbs);
}

static int series6_send_suswarn(struct platform_intf *intf)
{
	struct gpio_map gpio30 = {
		.type	= GPIO_OUT,
		.port	= 0,
		.pin	= 30,
	};
	uint16_t gpio_base;
	uint32_t val;

	if (ich_get_gpio_base(intf, &gpio_base) < 0)
		return -1;

	/* set state of GPIO30 as GPIO output and drive low */
	series6_set_gpio(intf, &gpio30, 0);

	/* set GPIO30 to reset by RSMRST only */
	if (io_read32(intf, gpio_base, &val) < 0)
		return -1;;
	if (io_write32(intf, gpio_base + 0x60, val | (1 << 30)) < 0)
		return -1;

	return 0;
}

int series6_global_reset(struct platform_intf *intf)
{
	/* Send SUSWARN#, especially if platform has an EC */
	if (series6_send_suswarn(intf))
		return -1;

	/* if it returns, something went wrong */
	return ich_global_reset(intf);
}
