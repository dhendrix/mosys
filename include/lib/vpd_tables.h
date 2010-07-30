/*
 * Copyright (C) 2010 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef MOSYS_LIB_VPD_TABLES_H__
#define MOSYS_LIB_VPD_TABLES_H__

#include <inttypes.h>

#include "mosys/string.h"

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
