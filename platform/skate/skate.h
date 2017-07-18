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

#ifndef PLATFORM_SKATE_H__
#define PLATFORM_SKATE_H__

#include <inttypes.h>
#include "mosys/platform.h"

/* FIXME: verify the board IDs against schematic */
#define SKATE_BOARD_REV0	"BOARD_REV0"
#define SKATE_BOARD_REV1	"BOARD_REV1"
#define SKATE_BOARD_REV2	"BOARD_REV2"

enum skate_board_config {
	SKATE_CONFIG_UNKNOWN = -1,
	SKATE_CONFIG_PROTO_MICRON,
	SKATE_CONFIG_PROTO_HYNIX,
	SKATE_CONFIG_PROTO_ELPIDA,
	SKATE_CONFIG_EVT_MICRON,
	SKATE_CONFIG_EVT_HYNIX,
	SKATE_CONFIG_EVT_ELPIDA,
	SKATE_CONFIG_DVT_MICRON,
	SKATE_CONFIG_DVT_HYNIX,
	SKATE_CONFIG_DVT_ELPIDA,
	SKATE_CONFIG_PVT_MICRON,
	SKATE_CONFIG_PVT_HYNIX,
	SKATE_CONFIG_PVT_ELPIDA,
	SKATE_CONFIG_MP_MICRON,
	SKATE_CONFIG_MP_HYNIX,
	SKATE_CONFIG_MP_ELPIDA,
	SKATE_CONFIG_RSVD,
};

extern enum skate_board_config skate_board_config;
extern int skate_ec_setup(struct platform_intf *intf);

/* platform callbacks */
extern struct eeprom_cb skate_eeprom_cb;	/* eeprom.c */
extern struct sys_cb skate_sys_cb;		/* sys.c */
extern struct gpio_cb skate_gpio_cb;		/* gpio.c */
extern struct memory_cb skate_memory_cb;	/* memory.c */
extern struct nvram_cb cros_ec_nvram_cb;	/* drivers/google/cros_ec.c */

#endif /* PLATFORM_SKATE_H_ */
