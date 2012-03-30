/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * gec.h: Subset of Google EC interface functionality (ported from chromium
 * os repo).
 */

#ifndef MOSYS_DRIVERS_EC_GOOGLE_H__
#define MOSYS_DRIVERS_EC_GOOGLE_H__

struct platform_intf;

/* I/O addresses for LPC commands */
#define EC_LPC_ADDR_KERNEL_DATA   0x62
#define EC_LPC_ADDR_KERNEL_CMD    0x66
#define EC_LPC_ADDR_KERNEL_PARAM 0x800
#define EC_LPC_ADDR_USER_DATA    0x200
#define EC_LPC_ADDR_USER_CMD     0x204
#define EC_LPC_ADDR_USER_PARAM   0x880
#define EC_LPC_PARAM_SIZE          128  /* Size of each param area in bytes */

/* LPC command status byte masks */
/* EC has written a byte in the data register and host hasn't read it yet */
#define EC_LPC_STATUS_TO_HOST     0x01
/* Host has written a command/data byte and the EC hasn't read it yet */
#define EC_LPC_STATUS_FROM_HOST   0x02
/* EC is processing a command */
#define EC_LPC_STATUS_PROCESSING  0x04
/* Last write to EC was a command, not data */
#define EC_LPC_STATUS_LAST_CMD    0x08
/* EC is in burst mode.  Chrome EC doesn't support this, so this bit is never
 * set. */
#define EC_LPC_STATUS_BURST_MODE  0x10
/* SCI event is pending (requesting SCI query) */
#define EC_LPC_STATUS_SCI_PENDING 0x20
/* SMI event is pending (requesting SMI query) */
#define EC_LPC_STATUS_SMI_PENDING 0x40
/* (reserved) */
#define EC_LPC_STATUS_RESERVED    0x80

/* EC is busy.  This covers both the EC processing a command, and the host has
 * written a new command but the EC hasn't picked it up yet. */
#define EC_LPC_STATUS_BUSY_MASK \
	(EC_LPC_STATUS_FROM_HOST | EC_LPC_STATUS_PROCESSING)

/* LPC command response codes */
/* TODO: move these so they don't overlap SCI/SMI data? */
enum lpc_status {
	EC_LPC_RESULT_SUCCESS = 0,
	EC_LPC_RESULT_INVALID_COMMAND = 1,
	EC_LPC_RESULT_ERROR = 2,
	EC_LPC_RESULT_INVALID_PARAM = 3,
};

/* Hello.  This is a simple command to test the EC is responsive to
 * commands. */
#define EC_LPC_COMMAND_HELLO 0x01
struct lpc_params_hello {
	uint32_t in_data;  /* Pass anything here */
} __attribute__ ((packed));
struct lpc_response_hello {
	uint32_t out_data;  /* Output will be in_data + 0x01020304 */
} __attribute__ ((packed));

/* Get version number */
#define EC_LPC_COMMAND_GET_VERSION 0x02
enum lpc_current_image {
	EC_LPC_IMAGE_UNKNOWN = 0,
	EC_LPC_IMAGE_RO,
	EC_LPC_IMAGE_RW_A,
	EC_LPC_IMAGE_RW_B
};

#define EC_LPC_COMMAND_GET_VERSION 0x02
struct lpc_response_get_version {
	/* Null-terminated version strings for RO, RW-A, RW-B */
	char version_string_ro[32];
	char version_string_rw_a[32];
	char version_string_rw_b[32];
	uint32_t current_image;  /* One of lpc_current_image */
} __attribute__ ((packed));

/* Get build information */
#define EC_LPC_COMMAND_GET_BUILD_INFO 0x04
struct lpc_response_get_build_info {
	char build_string[EC_LPC_PARAM_SIZE];
} __attribute__ ((packed));

extern int gec_hello(struct platform_intf *intf);
const char *gec_version(struct platform_intf *intf);

#endif	/* MOSYS_DRIVERS_EC_GOOGLE__ */
