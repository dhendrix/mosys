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

#ifndef EXPERIMENTAL_STOUT_H__
#define EXPERIMENTAL_STOUT_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define STOUT_HOST_FIRMWARE_ROM_SIZE	(8192 * 1024)

/* These are firmware-specific and not generically useful for it8500 */
typedef enum stout_ec_command {
        STOUT_ECCMD_MEM_READ                            = 0x08,
        STOUT_ECCMD_MEM_WRITE                           = 0x09,
        STOUT_ECCMD_GET_BATTERY_AUTH_STATUS             = 0x20,
        STOUT_ECCMD_GET_BATTERY_FIRST_USE_DATE_STATUS   = 0x21,
        STOUT_ECCMD_LATCH_BATTERY_FIRST_USE_DATE        = 0x22,
        STOUT_ECCMD_GET_BATTERY_FIRST_USE_DATE_HI       = 0x23,
        STOUT_ECCMD_GET_BATTERY_FIRST_USE_DATE_LO       = 0x24,
	STOUT_ECCMD_BATTERY_FW_UPDATE_COMPLETION_STATUS = 0x40,
	STOUT_ECCMD_BATTERY_FW_UPDATE_NEEDED            = 0x41,
	STOUT_ECCMD_BATTERY_FW_UPDATE_STATUS            = 0x42,
} stout_ec_command;

typedef enum stout_ec_mem_addr {
        STOUT_ECMEM_BATTERY_STATUS              = 0x38,
	STOUT_ECMEM_BATTERY_FW_UPDATE           = 0x3b,
        STOUT_ECMEM_FW_VERSION_MSB              = 0xe8,
        STOUT_ECMEM_FW_VERSION_LSB              = 0xe9,
        STOUT_ECMEM_BATTERY_FIRST_USE_DATE_HI   = 0xbd,
        STOUT_ECMEM_BATTERY_FIRST_USE_DATE_LO   = 0xbe,
} stout_ec_mem_addr;

/* platform callbacks */
extern struct battery_cb stout_battery_cb;	/* battery.c */
extern struct ec_cb stout_ec_cb;		/* ec.c */
extern struct eeprom_cb stout_eeprom_cb;	/* eeprom.c */
extern struct memory_cb stout_memory_cb;	/* memory.c */
extern struct nvram_cb stout_nvram_cb;		/* nvram.c */
extern struct sys_cb stout_sys_cb;		/* sys.c */

/* functions called by setup routines */
extern int stout_ec_setup(struct platform_intf *intf);
extern int stout_eeprom_setup(struct platform_intf *intf);

/* ec functions used by ec and battery commands */
extern int ec_command(struct platform_intf *intf, stout_ec_command command,
                        uint8_t *input_data, uint8_t input_len,
                        uint8_t *output_data, uint8_t output_len );
extern int ecram_read(struct platform_intf *intf,
                        stout_ec_mem_addr address, uint8_t *data,
			stout_ec_command cmd);
extern int ecram_write(struct platform_intf *intf,
                        stout_ec_mem_addr address, uint8_t data);

#endif /* EXPERIMENTAL_STOUT_H_ */
