/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
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
 * cros_ec.h: Subset of ChromeOS EC interface functionality (ported from
 * Chromium OS repo).
 */

#ifndef MOSYS_DRIVERS_EC_GOOGLE_H__
#define MOSYS_DRIVERS_EC_GOOGLE_H__

#include "intf/i2c.h"

struct eeprom;
struct platform_intf;
struct ec_response_get_chip_info;
struct ec_response_flash_info;

struct cros_ec_priv {
	/* Wrapped with EC lock */
	int (*cmd)(struct platform_intf *intf, struct ec_cb *ec,
		   int command, int command_version,
		   const void *indata, int insize,
		   const void *outdata, int outsize);
	/* Low level command function, bus and version specific */
	int (*raw)(struct platform_intf *intf, struct ec_cb *ec,
		   int command, int command_version,
		   const void *indata, int insize,
		   const void *outdata, int outsize);

	union {
		struct i2c_addr i2c;
		uint16_t io;
		int fd;
	} addr;
	int device_index;
	char* device_name;
};

extern struct ec_cb cros_ec_cb;
extern struct ec_cb cros_pd_cb;
extern struct ec_cb cros_sh_cb;

/* EC commands */
extern int cros_ec_hello(struct platform_intf *intf, struct ec_cb *ec);
const char *cros_ec_version(struct platform_intf *intf, struct ec_cb *ec);
extern int cros_ec_chip_info(struct platform_intf *intf, struct ec_cb *ec,
		         struct ec_response_get_chip_info *info);
extern int cros_ec_flash_info(struct platform_intf *intf, struct ec_cb *ec,
		         struct ec_response_flash_info *info);
extern int cros_ec_detect(struct platform_intf *intf, struct ec_cb *ec);
extern int cros_ec_board_version(struct platform_intf *intf, struct ec_cb *ec);

/*
 * This is intended to be used in platform-specific system callbacks (sys_cb)
 * which means it also allocates memory that must be freed. For these callbacks
 * the high-level command handling code is typically responsible for calling
 * the function and freeing the allocated memory.
 */
extern char *cros_ec_board_version_str(struct platform_intf *intf);

extern int cros_ec_vbnvcontext_read(struct platform_intf *intf,
		struct ec_cb *ec, uint8_t *block);
extern int cros_ec_vbnvcontext_write(struct platform_intf *intf,
		struct ec_cb *ec, const uint8_t *block);
extern int cros_ec_get_firmware_rom_size(struct platform_intf *intf);


extern int cros_ec_probe_dev(struct platform_intf *intf, struct ec_cb *ec);
extern int cros_ec_probe_i2c(struct platform_intf *intf);
extern int cros_ec_probe_lpc(struct platform_intf *intf);

/* PD commands */
extern int cros_pd_probe_lpc(struct platform_intf *intf);
extern int cros_pd_hello(struct platform_intf *intf);
const char *cros_pd_version(struct platform_intf *intf);
extern int cros_pd_chip_info(struct platform_intf *intf,
		         struct ec_response_get_chip_info *info);
extern int cros_pd_flash_info(struct platform_intf *intf,
		         struct ec_response_flash_info *info);
extern int cros_pd_detect(struct platform_intf *intf);

#define CROS_EC_DEV_NAME		"/dev/cros_ec"
#define CROS_PD_DEV_NAME		"/dev/cros_pd"
#define CROS_SH_DEV_NAME		"/dev/cros_sh"

#endif	/* MOSYS_DRIVERS_EC_GOOGLE__ */
