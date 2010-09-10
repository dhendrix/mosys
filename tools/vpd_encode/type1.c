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

/**
 * vpd_append_type1 - append type 1 (system info) structure
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int vpd_append_type1(uint16_t handle, uint8_t **buf, size_t len)
{
	struct vpd_header *header;
	struct vpd_table_system *data;
	uint8_t *strings;
	size_t struct_len, total_len;

	/* FIXME: Add sanity checking */
	struct_len = sizeof(struct vpd_header) +
	             sizeof(struct vpd_table_system) +
	             strlen(CONFIG_SYSTEM_MANUFACTURER) + 1 +
	             strlen(CONFIG_SYSTEM_PRODUCT_NAME) + 1 +
	             strlen(CONFIG_SYSTEM_VERSION) + 1 +
	             strlen(CONFIG_SYSTEM_SERIAL_NUMBER) + 1 +
	             strlen(CONFIG_SYSTEM_SKU) + 1 +
	             strlen(CONFIG_SYSTEM_FAMILY) + 1 +
		     1;			/* structure terminator */
	total_len = len + struct_len;

	*buf = realloc(*buf, total_len);
	memset(*buf + len, 0, struct_len);

	header = *buf + len;
	data = (uint8_t *)header + sizeof(*header);
	strings = (uint8_t *)data + sizeof(*data);

	/* fill in structure header details */
	header->type = VPD_TYPE_SYSTEM;
	header->length = sizeof(*header) + sizeof(*data);
	header->handle = handle;

	data->manufacturer = 1;
	data->name = 2;
	data->version = 3;
	data->serial_number = 4;
#ifdef CONFIG_SYSTEM_UUID
	if (uuid_parse(CONFIG_SYSTEM_UUID, data->uuid) < 0) {
		printf("invalid UUID \"%s\" specified\n", CONFIG_SYSTEM_UUID);
		goto vpd_create_type1_fail;
	}
#endif
	data->wakeup_type = 0;	/* FIXME: we're basically ignoring this */
	data->sku_number = 5;
	data->family = 6;

	sprintf(strings, "%s%c%s%c%s%c%s%c%s%c%s%c",
	                 CONFIG_SYSTEM_MANUFACTURER, '\0',
	                 CONFIG_SYSTEM_PRODUCT_NAME, '\0',
	                 CONFIG_SYSTEM_VERSION, '\0',
	                 CONFIG_SYSTEM_SERIAL_NUMBER, '\0',
	                 CONFIG_SYSTEM_SKU, '\0',
	                 CONFIG_SYSTEM_FAMILY, '\0');

	memset(*buf + struct_len, 0, 1);	/* terminator */

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)total_len);
	print_buf(LOG_DEBUG, data, total_len);

	return total_len;

vpd_create_type1_fail:
	return -1;
}
