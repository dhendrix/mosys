/*
 * Copyright 2010 Google Inc.  All Rights Reserved.
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
 *
 * FIXME: this code is absolutely atrocious and needs to be re-written
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <uuid/uuid.h>

#include "mosys/log.h"

#include "lib/math.h"
#include "lib/string.h"
#include "lib/vpd.h"
#include "lib/vpd_tables.h"

#include "lib_vpd_encode.h"

/**
 * vpd_append_type0 - append type 0 (firmware info) structure to a buffer
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int vpd_append_type0(uint16_t handle, uint8_t **buf, size_t len)
{
	struct vpd_header *header;
	struct vpd_table_firmware *data;
	uint8_t *strings;
	size_t struct_len, total_len;

	/* FIXME: Add sanity checking */
	struct_len = sizeof(struct vpd_header) +
	             sizeof(struct vpd_table_firmware) +
	             strlen(CONFIG_FIRMWARE_VENDOR) + 1 +
	             strlen(CONFIG_FIRMWARE_VERSION) + 1 +
	             strlen(CONFIG_FIRMWARE_RELEASE_DATE) + 1 +
		     1;			/* structure terminator */
	total_len = len + struct_len;

	*buf = realloc(*buf, total_len);
	memset(*buf + len, 0, struct_len);

	header = *buf + len;
	data = (uint8_t *)header + sizeof(*header);
	strings = (uint8_t *)data + sizeof(*data);

	/* fill in structure header details */
	header->type = VPD_TYPE_FIRMWARE;
	header->length = sizeof(*header) + sizeof(*data);
	header->handle = handle;

	data->vendor = 1;
	data->version = 2;
#ifdef CONFIG_FIRMWARE_START_ADDRESS
	data->start_address = CONFIG_FIRMWARE_START_ADDRESS;
#else
	data->start_address = 0;
#endif
	data->rom_size_64k_blocks = CONFIG_FIRMWARE_ROM_SIZE - 1;
	data->major_ver = CONFIG_SYSTEM_FIRMWARE_MAJOR_RELEASE;
	data->minor_ver = CONFIG_SYSTEM_FIRMWARE_MINOR_RELEASE;
	data->ec_major_ver = CONFIG_EC_FIRMWARE_MAJOR_RELEASE;
	data->ec_minor_ver = CONFIG_EC_FIRMWARE_MINOR_RELEASE;

	data->release_date = 3;
	sprintf(strings, "%s%c%s%c%s%c",
	                 CONFIG_FIRMWARE_VENDOR, '\0',
	                 CONFIG_FIRMWARE_VERSION, '\0',
	                 CONFIG_FIRMWARE_RELEASE_DATE, '\0');

#ifdef CONFIG_ADVANCED_OPTIONS
#ifdef CONFIG_FIRMWARE_START_SEGMENT
	data->start_address = (uint16_t)CONFIG_FIRMWARE_START_SEGMENT;
#endif
	/* FIXME: add code to handle characteristics + extension */
#endif	/* CONFIG_ADVANCED_OPTIONS */

	memset(*buf + struct_len, 0, 1);	/* terminator */

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)total_len);
	print_buf(LOG_DEBUG, data, total_len);
	return total_len;
}

