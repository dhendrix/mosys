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

/* print buffer (for debugging) */
void print_buf(enum log_levels threshold, void *buf, size_t len)
{
	size_t i;
	uint8_t *x;

	if (CONFIG_LOGLEVEL < threshold)
		return;

	for (i = 0; i < len; i++) {
		x = (uint8_t *)buf + i;
		if ((i % 16 == 0) && (i != 0))
			lprintf(threshold, "\n");
		lprintf(threshold, "%02x ", *x);
	}
	lprintf(threshold, "\n");
}

/*
 * This function simply looks for the pattern of two adjacent NULL bytes
 * following the table header.
 */
int vpd_sizeof_strings(void *table)
{
	uint8_t *p;
	struct vpd_header *header = table;
	size_t size = 0, offset = 0;
	unsigned char cmp[2] = { '\0', '\0' };
	uint8_t found = 0;

	/*
	 * Search for double NULL. End of strings will be one byte before the
	 * final terminator which indicates end of structure.
	 */
	for (p = table + header->length - 1; p; p++, offset++) {
		if (!memcmp(p, cmp, 2)) {
			found = 1;
			break;
		}
	}

	if (found)
		size = offset;

	return size;
}

#if 0
/* returns updated length of strings, <0 to indicate failure */
int vpd_append_string()
{
}
#endif

/**
 * vpd_create_eps - create an entry point structure
 *
 * @structure_table_len:	structure table len
 * @num_structures:		number of structures in structure table
 *
 * As per SMBIOS spec, the caller must place this structure on a 16-byte
 * boundary so that the anchor strings can be found.
 *
 * returns pointer to newly allocated entry point structure if successful
 * returns NULL to indicate failure
 *
 * FIXME: This function needs to be more intelligent about parsing tables and
 * obtaining information on its own. These arguments need to go away.
 */
struct vpd_entry *vpd_create_eps(uint8_t major_ver,
                                 uint8_t minor_ver,
                                 uint16_t structure_table_length,
                                 uint32_t structure_table_address,
                                 uint16_t num_structures)
{
	struct vpd_entry *eps = NULL;

	/* size of structure only, no strings */
	eps = malloc(sizeof(struct vpd_entry));
	if (!eps)
		return NULL;
	memset(eps, 0, sizeof(*eps));

	memcpy(eps->anchor_string, VPD_ENTRY_MAGIC, 4);
	/* Note: entry point length should be 0x1F for v2.6 */
	eps->entry_length = sizeof(struct vpd_entry);
	eps->major_ver = major_ver;
	eps->minor_ver = minor_ver;
	/* EPS revision based on version 2.1 or later */
	eps->entry_rev = 0;
	/* note: nothing done with EPS formatted area */

	/* Intermediate EPS (IEPS) stuff */
	memcpy(eps->inter_anchor_string, "_DMI_", 5);

	eps->table_length = structure_table_length;
	eps->table_address = structure_table_address;
	eps->table_entry_count = num_structures;
	eps->bcd_revision = (major_ver << 4) | minor_ver;

	/* calculate IEPS checksum first, then the EPS checksum */
	eps->inter_anchor_cksum = zero8_csum(&eps->inter_anchor_string[0], 0xf);
	eps->entry_cksum = zero8_csum((uint8_t *)eps, eps->entry_length);

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)eps->entry_length);
	print_buf(LOG_DEBUG, eps, eps->entry_length);

	return eps;
}

/**
 * vpd_append_type127 - append type 127 (end of table) structure
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int vpd_append_type127(uint16_t handle, uint8_t **buf, size_t len)
{
	struct vpd_table_eot *data;
	size_t total_len, struct_len;
	uint8_t *p;

	struct_len = sizeof(struct vpd_table_eot) + 2;	/* double terminator */
	total_len = len + struct_len;
	*buf = realloc(*buf, total_len);

	p = *buf + len;
	data = (struct vpd_table_eot *)p;
	data->header.type = 127;
	data->header.length = sizeof(*data);
	data->header.handle = handle;

	memset(*buf + len + sizeof(*data), 0, 2);	/* double terminator */

	return total_len;
}

/**
 * vpd_append_type241 - append type 241 (binary blob pointer) structure
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 * @vendor:	blob vendor string
 * @desc:	blob description string
 * @variant:	blob variant string
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int vpd_append_type241(uint16_t handle, uint8_t **buf,
                       size_t len, const char *uuid, uint32_t offset,
                       uint32_t size, const char *vendor,
                       const char *desc, const char *variant)
{
	struct vpd_header *header;
	struct vpd_table_binary_blob_pointer *data;
	uint8_t *string_ptr, *p;
	size_t struct_len, total_len;
	int string_index = 1;

	/* FIXME: Add sanity checking */
	struct_len = sizeof(struct vpd_header) +
	             sizeof(struct vpd_table_binary_blob_pointer);
	if (vendor)
		struct_len += strlen(vendor) + 1;
	if (desc)
		struct_len += strlen(desc) + 1;
	if (variant)
		struct_len += strlen(variant) + 1;
	struct_len += 1;	/* structure terminator */
	total_len = len + struct_len;

	*buf = realloc(*buf, total_len);
	memset(*buf + len, 0, struct_len);

	p = *buf + len;
	header = (struct vpd_header *)p;
	p += sizeof(*header);
	data = (struct vpd_table_binary_blob_pointer *)p;
	p += sizeof(*data);
	string_ptr = p;

	/* fill in structure header details */
	header->type = VPD_TYPE_BINARY_BLOB_POINTER;
	header->length = sizeof(*header) + sizeof(*data);
	header->handle = handle;

	data->struct_major_version = 1;
	data->struct_minor_version = 0;

	if (vendor) {
		data->vendor = string_index;
		string_index++;
		sprintf(string_ptr, "%s%c", vendor, '\0');
		string_ptr += strlen(vendor) + 1;
	}

	if (desc) {
		data->description = 2;
		string_index++;
		sprintf(string_ptr, "%s%c", desc, '\0');
		string_ptr += strlen(desc) + 1;
	}

	data->major_version = 0;
	data->minor_version = 1;

	if (variant) {
		data->variant = string_index;
		string_index++;
		sprintf(string_ptr, "%s%c", variant, '\0');
		string_ptr += strlen(variant) + 1;
	}

	memset(&data->reserved[0], 0, 5);

	if (uuid_parse(uuid, &data->uuid[0]) < 0) {
		printf("invalid UUID \"%s\" specified\n", uuid);
		goto vpd_create_type241_fail;
	}

	data->offset = offset;
	data->size = size;

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)total_len);
	print_buf(LOG_DEBUG, data, total_len);

	return total_len;

vpd_create_type241_fail:
	return -1;
}

void vpd_free_table(void *data)
{
	uint8_t *foo = data;

	/* clean-up is trivially simple, for now... */
	free(foo);
}
