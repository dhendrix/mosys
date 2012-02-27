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

#ifndef MOSYS_LIB_VPD_TABLES_H__
#define MOSYS_LIB_VPD_TABLES_H__

#include <inttypes.h>

#include "lib/string.h"

#define VPD_ENTRY_MAGIC		"_SM_"

/* Entry */
struct vpd_entry {
	uint8_t anchor_string[4];
	uint8_t entry_cksum;
	uint8_t entry_length;
	uint8_t major_ver;
	uint8_t minor_ver;
	uint16_t max_size;
	uint8_t entry_rev;
	uint8_t format_area[5];
	uint8_t inter_anchor_string[5];
	uint8_t inter_anchor_cksum;
	uint16_t table_length;
	uint32_t table_address;
	uint16_t table_entry_count;
	uint8_t bcd_revision;
} __attribute__ ((packed));

/* Header */
struct vpd_header {
	uint8_t type;
	uint8_t length;
	uint16_t handle;
} __attribute__ ((packed));

/* Type 0 - firmware information */
struct vpd_table_firmware {
	uint8_t vendor;
	uint8_t version;
	uint16_t start_address;
	uint8_t release_date;
	uint8_t rom_size_64k_blocks;
	uint32_t characteristics;
	uint8_t extension[2];	/* v2.4+ */
	uint8_t major_ver;	/* v2.4+ */
	uint8_t minor_ver;	/* v2.4+ */
	uint8_t ec_major_ver;	/* v2.4+ */
	uint8_t ec_minor_ver;	/* v2.4+ */
} __attribute__ ((packed));

/* Type 1 - system information */
struct vpd_table_system {
	uint8_t manufacturer;
	uint8_t name;
	uint8_t version;
	uint8_t serial_number;
	uint8_t uuid[16];
	uint8_t wakeup_type;
	uint8_t sku_number;	/* v2.4+ */
	uint8_t family;		/* v2.4+ */
} __attribute__ ((packed));

/* Type 127 - end of table */
struct vpd_table_eot {
	struct vpd_header header;
} __attribute__ ((packed));

/* Type 241 - binary blob pointer */
struct vpd_table_binary_blob_pointer {
	uint8_t struct_major_version;
	uint8_t struct_minor_version;
	uint8_t vendor;
	uint8_t description;
	uint8_t major_version;
	uint8_t minor_version;
	uint8_t variant;
	uint8_t reserved[5];
	uint8_t uuid[16];
	uint32_t offset;
	uint32_t size;
} __attribute__ ((packed));

/* The length and number of strings defined here is not a limitation of VPD.
 * These numbers were deemed good enough during development. */
#define VPD_MAX_STRINGS 10
#define VPD_MAX_STRING_LENGTH 64

/* One structure to rule them all */
struct vpd_table {
	struct vpd_header header;
	union {
		struct vpd_table_firmware firmware;
		struct vpd_table_system system;
		struct vpd_table_binary_blob_pointer blob;
		uint8_t data[1024];
	} data;
	char string[VPD_MAX_STRINGS][VPD_MAX_STRING_LENGTH];
};

#endif /* MOSYS_LIB_VPD_TABLES_H__ */
