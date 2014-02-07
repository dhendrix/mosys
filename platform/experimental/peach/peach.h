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

#ifndef PLATFORM_PEACH_H__
#define PLATFORM_PEACH_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define PEACH_BOARD_REV0	"BOARD_REV0"
#define PEACH_BOARD_REV1	"BOARD_REV1"
#define PEACH_BOARD_REV2	"BOARD_REV2"
#define PEACH_BOARD_REV3	"BOARD_REV3"

enum peach_board_config {
	PEACH_CONFIG_UNKNOWN = -1,

	PEACH_PIT_CONFIG_RSVD = 0,
	PEACH_PIT_CONFIG_REV_0_0,
	PEACH_PIT_CONFIG_REV_3_0,
	PEACH_PIT_CONFIG_REV_4_0,
	PEACH_PIT_CONFIG_REV_5_0,
	PEACH_PIT_CONFIG_REV_6_0,
	PEACH_PIT_CONFIG_REV_7_0,
	PEACH_PIT_CONFIG_REV_7_2,
	PEACH_PIT_CONFIG_REV_9_0,
	PEACH_PIT_CONFIG_REV_9_2,
	PEACH_PIT_CONFIG_REV_A_0,
	PEACH_PIT_CONFIG_REV_A_2,
	PEACH_PIT_CONFIG_REV_B_0,
	PEACH_PIT_CONFIG_REV_B_2,
	PEACH_PIT_CONFIG_REV_C_0,
	PEACH_PIT_CONFIG_REV_C_2,
	PEACH_PIT_CONFIG_REV_D_0,
	PEACH_PIT_CONFIG_REV_D_1,
	PEACH_PIT_CONFIG_REV_D_2,
	PEACH_PIT_CONFIG_REV_D_3,
	PEACH_PIT_CONFIG_REV_E_0,
	PEACH_PIT_CONFIG_REV_E_1,
	PEACH_PIT_CONFIG_REV_E_2,
	PEACH_PIT_CONFIG_REV_E_3,
	PEACH_PIT_CONFIG_REV_F_0,
	PEACH_PIT_CONFIG_REV_F_1,
	PEACH_PIT_CONFIG_REV_F_2,
	PEACH_PIT_CONFIG_REV_F_3,
	PEACH_PIT_CONFIG_REV_10_0,
	PEACH_PIT_CONFIG_REV_10_1,
	PEACH_PIT_CONFIG_REV_10_2,
	PEACH_PIT_CONFIG_REV_10_3,

	PEACH_KIRBY_CONFIG_RSVD,
	PEACH_KIRBY_CONFIG_PROTO0,
	PEACH_KIRBY_CONFIG_PROTO1,
	PEACH_KIRBY_CONFIG_EVT,
	PEACH_KIRBY_CONFIG_DVT,
	PEACH_KIRBY_CONFIG_PVT,
	PEACH_KIRBY_CONFIG_MP,

	PEACH_PI_CONFIG_RSVD,
	PEACH_PI_CONFIG_REV_8_4,
	PEACH_PI_CONFIG_REV_9_4,
	PEACH_PI_CONFIG_REV_A_6,
	PEACH_PI_CONFIG_REV_B_6,
	PEACH_PI_CONFIG_REV_C_6,
	PEACH_PI_CONFIG_REV_D_4,
	PEACH_PI_CONFIG_REV_D_5,
	PEACH_PI_CONFIG_REV_D_6,
	PEACH_PI_CONFIG_REV_D_7,
	PEACH_PI_CONFIG_REV_E_4,
	PEACH_PI_CONFIG_REV_E_5,
	PEACH_PI_CONFIG_REV_E_6,
	PEACH_PI_CONFIG_REV_E_7,
	PEACH_PI_CONFIG_REV_F_4,
	PEACH_PI_CONFIG_REV_F_5,
	PEACH_PI_CONFIG_REV_F_6,
	PEACH_PI_CONFIG_REV_F_7,
	PEACH_PI_CONFIG_REV_10_4,
	PEACH_PI_CONFIG_REV_10_5,
	PEACH_PI_CONFIG_REV_10_6,
	PEACH_PI_CONFIG_REV_10_7,
};

extern enum peach_board_config peach_board_config;
extern int peach_ec_setup(struct platform_intf *intf);

/* platform callbacks */
extern struct eeprom_cb peach_eeprom_cb;	/* eeprom.c */
extern struct sys_cb peach_sys_cb;		/* sys.c */
extern struct gpio_cb peach_gpio_cb;		/* gpio.c */
extern struct memory_cb peach_memory_cb;	/* memory.c */
extern struct nvram_cb cros_ec_nvram_cb;	/* drivers/google/cros_ec.c */

#endif /* PLATFORM_PEACH_H_ */
