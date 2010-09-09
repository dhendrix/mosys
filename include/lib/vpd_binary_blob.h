/* Copyright 2010, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *    * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
 * vpd_binary_blob.h: helper functions for handling binary blobs
 */

#ifndef MOSYS_LIB_VPD_BINARY_BLOB_H__
#define MOSYS_LIB_VPD_BINARY_BLOB_H__

#include <uuid/uuid.h>

#include "mosys/kv_pair.h"

struct agz_blob_0_3 {
	/* Sample AGZ blob version 3 */
	uint8_t product_name[16];
	uint8_t product_manufacturer[16];
	uint8_t uuid[16];
	uint8_t motherboard_serial_number[16];
	uint8_t esn_3g[10];
	uint8_t country_code[6];
	uint8_t wlan_mac_id[6];
	uint8_t reserved_1[10];
	uint8_t product_serial_number[22];
} __attribute__ ((packed));

struct agz_blob_0_5 {
	/* Sample AGZ blob version 5 */
	uint8_t product_name[16];
	uint8_t product_manufacturer[16];
	uint8_t uuid[16];
	uint8_t motherboard_serial_number[32];
	uint8_t esn_3g[10];
	uint8_t country_code[6];
	uint8_t wlan_mac_id[6];
	uint8_t reserved_1[10];
	uint8_t product_serial_number[22];
} __attribute__ ((packed));

struct google_blob_1_1 {
	/* Google VPD blob version 1.1 */
	uint8_t product_serial_number[32];	/* ASCII */
	uint8_t product_sku[16];		/* ASCII */
	uint8_t uuid[16];			/* raw binary */
	uint8_t motherboard_serial_number[16];	/* ASCII */
	uint8_t imei_3g[16];			/* ASCII */
	uint8_t ssd_serial_number[16];		/* ASCII */
	uint8_t memory_serial_number[16];	/* ASCII */
	uint8_t wlan_mac_id[6];			/* raw binary */
} __attribute__ ((packed));

struct blob_handler {
	unsigned char *uuid;	/* string representation of UUID */
	int (*print)(uint8_t *blob, uint32_t size, struct kv_pair *kv);
};

int print_agz_blob_v3(uint8_t *data, uint32_t size, struct kv_pair *kv);
int print_agz_blob_v5(uint8_t *data, uint32_t size, struct kv_pair *kv);
int print_google_blob_v1_1(uint8_t *data, uint32_t size, struct kv_pair *kv);

#endif	/* MOSYS_LIB_VPD_BINARY_BLOB_H__ */
