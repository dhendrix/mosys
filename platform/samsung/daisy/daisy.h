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

#ifndef PLATFORM_DAISY_H__
#define PLATFORM_DAISY_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define SNOW_BOARD_ID0	"ID_Bit0"
#define SNOW_BOARD_ID1	"ID_Bit1"

/* board config is used for determining DRAM vendor and platform revision */
enum daisy_board_config {
	/* TODO: add Daisy configurations here as well */
	SNOW_CONFIG_UNKNOWN = -1,
	SNOW_CONFIG_SAMSUNG_EVT,
	SNOW_CONFIG_ELPIDA_EVT,
	SNOW_CONFIG_SAMSUNG_DVT,
	SNOW_CONFIG_ELPIDA_DVT,
	SNOW_CONFIG_SAMSUNG_PVT,
	SNOW_CONFIG_SAMSUNG_PVT2,
	SNOW_CONFIG_ELPIDA_PVT,
	SNOW_CONFIG_ELPIDA_PVT2,
	SNOW_CONFIG_SAMSUNG_MP,
	SNOW_CONFIG_ELPIDA_MP,
	SNOW_CONFIG_SAMSUNG_MP_1_2,
	SNOW_CONFIG_SAMSUNG_MP_2_0,
	SNOW_CONFIG_RSVD,
};

extern enum daisy_board_config board_config;
extern int daisy_ec_setup(struct platform_intf *intf);

/* platform callbacks */
extern struct eeprom_cb daisy_eeprom_cb;	/* eeprom.c */
extern struct sys_cb daisy_sys_cb;		/* sys.c */
extern struct gpio_cb daisy_gpio_cb;		/* gpio.c */
extern struct memory_cb daisy_memory_cb;	/* memory.c */
extern struct nvram_cb gec_nvram_cb;		/* drivers/google/gec.c */

#endif /* PLATFORM_DAISY_H_ */
