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
 * gec.h: Subset of Google EC interface functionality (ported from chromium
 * os repo).
 */

#ifndef MOSYS_DRIVERS_EC_GOOGLE_H__
#define MOSYS_DRIVERS_EC_GOOGLE_H__

#include "intf/i2c.h"

struct platform_intf;

#define GEC_PARAM_SIZE          128  /* Size of each param area in bytes */

/* GEC command response codes */
/* TODO: move these so they don't overlap SCI/SMI data? */
enum gec_status {
	GEC_RESULT_SUCCESS = 0,
	GEC_RESULT_INVALID_COMMAND = 1,
	GEC_RESULT_ERROR = 2,
	GEC_RESULT_INVALID_PARAM = 3,
};

/* Hello.  This is a simple command to test the EC is responsive to
 * commands. */
#define GEC_COMMAND_HELLO 0x01
struct gec_params_hello {
	uint32_t in_data;  /* Pass anything here */
} __attribute__ ((packed));
struct gec_response_hello {
	uint32_t out_data;  /* Output will be in_data + 0x01020304 */
} __attribute__ ((packed));

/* Get version number */
#define GEC_COMMAND_GET_VERSION 0x02
enum gec_current_image {
	GEC_IMAGE_UNKNOWN = 0,
	GEC_IMAGE_RO,
	GEC_IMAGE_RW_A,
	GEC_IMAGE_RW_B
};

#define GEC_COMMAND_GET_VERSION 0x02
struct gec_response_get_version {
	/* Null-terminated version strings for RO, RW-A, RW-B */
	char version_string_ro[32];
	char version_string_rw_a[32];
	char version_string_rw_b[32];
	uint32_t current_image;  /* One of gec_current_image */
} __attribute__ ((packed));

/* Get build information */
#define GEC_COMMAND_GET_BUILD_INFO 0x04
struct gec_response_get_build_info {
	char build_string[GEC_PARAM_SIZE];
} __attribute__ ((packed));

/* Get chip info */
#define GEC_CMD_GET_CHIP_INFO 0x05
struct gec_response_get_chip_info {
	/* Null-terminated strings */
	char vendor[32];
	char name[32];
	char revision[32];  /* Mask version */
} __attribute__ ((packed));

/* Get flash info */
#define GEC_CMD_FLASH_INFO 0x10
struct gec_response_flash_info {
	/* Usable flash size, in bytes */
	uint32_t flash_size;
	/* Write block size.  Write offset and size must be a multiple
	 * of this. */
	uint32_t write_block_size;
	/* Erase block size.  Erase offset and size must be a multiple
	 * of this. */
	uint32_t erase_block_size;
	/* Protection block size.  Protection offset and size must be a
	 * multiple of this. */
	uint32_t protect_block_size;
} __attribute__ ((packed));

struct gec_priv {
	/* the low-level command function depends on bus */
	int (*cmd)(struct platform_intf *intf, int command,
		   const void *indata, int insize,
		   const void *outdata, int outsize);

	union {
		struct i2c_addr i2c;
		uint16_t io;
	} addr;
};

extern struct ec_cb gec_cb;
extern int gec_hello(struct platform_intf *intf);
const char *gec_version(struct platform_intf *intf);
extern int gec_chip_info(struct platform_intf *intf,
		         struct gec_response_get_chip_info *info);
extern int gec_flash_info(struct platform_intf *intf,
		         struct gec_response_flash_info *info);
extern int gec_detect(struct platform_intf *intf);
extern int gec_probe_i2c(struct platform_intf *intf);
extern int gec_probe_lpc(struct platform_intf *intf);

#endif	/* MOSYS_DRIVERS_EC_GOOGLE__ */
