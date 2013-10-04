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

struct platform_intf;
struct ec_response_get_chip_info;
struct ec_response_flash_info;

struct cros_ec_priv {
	/* the low-level command function depends on bus */
	int (*cmd)(struct platform_intf *intf,
		   int command, int command_version,
		   const void *indata, int insize,
		   const void *outdata, int outsize);

	union {
		struct i2c_addr i2c;
		uint16_t io;
	} addr;
};

extern struct ec_cb cros_ec_cb;
extern int cros_ec_hello(struct platform_intf *intf);
const char *cros_ec_version(struct platform_intf *intf);
extern int cros_ec_chip_info(struct platform_intf *intf,
		         struct ec_response_get_chip_info *info);
extern int cros_ec_flash_info(struct platform_intf *intf,
		         struct ec_response_flash_info *info);
extern int cros_ec_detect(struct platform_intf *intf);
extern int cros_ec_probe_dev(struct platform_intf *intf);
extern int cros_ec_probe_i2c(struct platform_intf *intf);
extern int cros_ec_probe_lpc(struct platform_intf *intf);

extern int cros_ec_vbnvcontext_read(struct platform_intf *intf, uint8_t *block);
extern int cros_ec_vbnvcontext_write(struct platform_intf *intf,
				 const uint8_t *block);

#endif	/* MOSYS_DRIVERS_EC_GOOGLE__ */
