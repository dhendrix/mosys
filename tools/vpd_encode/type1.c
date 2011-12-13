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
#include "symbol.h"

/**
 * vpd_append_type1 - append type 1 (system info) structure
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * @product_name:	name of the product
 * @version:		version of the product
 * @serial_number:	serial number of the product
 * @uuid:		uuid of the product
 * @sku:		SKU of the product
 * @family:		family the product belongs in
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int vpd_append_type1(uint16_t handle, uint8_t **buf, size_t len,
                     char *manufacturer, char *product_name, char *version,
		     char *serial_number, char *uuid, char *sku, char *family)
{
	struct vpd_header *header;
	struct vpd_table_system *data;
	uint8_t *strings, *p;
	size_t struct_len, total_len;

	/* FIXME: Add sanity checking */
	struct_len = sizeof(struct vpd_header) +
	             sizeof(struct vpd_table_system) +
	             strlen(manufacturer) + 1 +
	             strlen(product_name) + 1 +
	             strlen(version) + 1 +
	             strlen(serial_number) + 1 +
	             strlen(sku) + 1 +
	             strlen(family) + 1 +
		     1;			/* structure terminator */
	total_len = len + struct_len;

	*buf = realloc(*buf, total_len);
	memset(*buf + len, 0, struct_len);

	p = *buf + len;
	header = (struct vpd_header *)p;
	p += sizeof(*header);
	data = (struct  vpd_table_system *)p;
	p += sizeof(*data);
	strings = p;

	/* fill in structure header details */
	header->type = VPD_TYPE_SYSTEM;
	header->length = sizeof(*header) + sizeof(*data);
	header->handle = handle;

	data->manufacturer = 1;
	data->name = 2;
	data->version = 3;
	data->serial_number = 4;
	if (uuid_parse(uuid, data->uuid) < 0)
		printf("invalid UUID \"%s\" specified\n", uuid);
	data->wakeup_type = 0; /* for legacy smbios compatibility only */
	data->sku_number = 5;
	data->family = 6;

	sprintf(strings, "%s%c%s%c%s%c%s%c%s%c%s%c",
	                 manufacturer, '\0',
	                 product_name, '\0',
	                 version, '\0',
	                 serial_number, '\0',
	                 sku, '\0',
	                 family, '\0');

	memset(*buf + struct_len, 0, 1);	/* terminator */

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)total_len);
	print_buf(LOG_DEBUG, data, total_len);

	return total_len;
}

/**
 * sym2type1 - extract symbols and append system info table to buffer
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * This function is intended to hide tedious symbol extraction steps from
 * higher-level logic.
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int sym2type1(uint16_t handle, uint8_t **buf, size_t len)
{
	char *manufacturer, *name, *version, *serial_num,
	     *uuid, *sku, *family;

	/* All strings are required in this data structure */
	if (!(manufacturer = sym2str("CONFIG_SYSTEM_MANUFACTURER"))) return -1;
	if (!(name = sym2str("CONFIG_SYSTEM_PRODUCT_NAME"))) return -1;
	if (!(version = sym2str("CONFIG_SYSTEM_VERSION"))) return -1;
	if (!(serial_num = sym2str("CONFIG_SYSTEM_SERIAL_NUMBER"))) return -1;
	if (!(uuid = sym2str("CONFIG_SYSTEM_UUID"))) return -1;
	if (!(sku = sym2str("CONFIG_SYSTEM_SKU"))) return -1;
	if (!(family = sym2str("CONFIG_SYSTEM_FAMILY"))) return -1;

	return vpd_append_type1(handle, buf, len, manufacturer, name,
	                        version, serial_num, uuid, sku, family);
}
