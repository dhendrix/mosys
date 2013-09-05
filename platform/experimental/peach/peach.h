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
	PEACH_PIT_CONFIG_PROTO,
	PEACH_PIT_CONFIG_EVT_2GB,
	PEACH_PIT_CONFIG_EVT_4GB,
	PEACH_PIT_CONFIG_DVT1_2GB,
	PEACH_PIT_CONFIG_DVT1_4GB,
	PEACH_PIT_CONFIG_DVT2_2GB,
	PEACH_PIT_CONFIG_DVT2_4GB,
	PEACH_PIT_CONFIG_PVT1_2GB,
	PEACH_PIT_CONFIG_PVT1_4GB,
	PEACH_PIT_CONFIG_PVT2_2GB,
	PEACH_PIT_CONFIG_PVT2_4GB,
	PEACH_PIT_CONFIG_MP_2GB,
	PEACH_PIT_CONFIG_MP_4GB,

	PEACH_KIRBY_CONFIG_RSVD,
	PEACH_KIRBY_CONFIG_PROTO0,
	PEACH_KIRBY_CONFIG_PROTO1,
	PEACH_KIRBY_CONFIG_EVT,
	PEACH_KIRBY_CONFIG_DVT,
	PEACH_KIRBY_CONFIG_PVT,
	PEACH_KIRBY_CONFIG_MP,
};

extern enum peach_board_config peach_board_config;
extern int peach_ec_setup(struct platform_intf *intf);

/* platform callbacks */
extern struct eeprom_cb peach_eeprom_cb;	/* eeprom.c */
extern struct sys_cb peach_sys_cb;		/* sys.c */
extern struct gpio_cb peach_gpio_cb;		/* gpio.c */
extern struct memory_cb peach_memory_cb;	/* memory.c */
extern struct nvram_cb gec_nvram_cb;		/* drivers/google/gec.c */

#endif /* PLATFORM_PEACH_H_ */
