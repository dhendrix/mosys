/*
 * Copyright 2010 Google Inc.  All Rights Reserved.
 * Author: David Hendricks (dhendrix@google.com)
 *
 * vpd.c: vpd routines
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
#include "mosys/string.h"

#include "lib/math.h"
#include "lib/vpd.h"
#include "lib/vpd_tables.h"

/* print buffer (for debugging) */
static void print_buf(enum log_levels threshold, void *buf, size_t len)
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
 * vpd_crete_eps - create an entry point structure
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
struct vpd_entry *vpd_create_eps(uint16_t structure_table_len,
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
	eps->major_ver = CONFIG_EPS_VPD_MAJOR_VERSION;
	eps->minor_ver = CONFIG_EPS_VPD_MINOR_VERSION;
	/* EPS revision based on version 2.1 or later */
	eps->entry_rev = 0;
	/* note: nothing done with EPS formatted area */

	/* Intermediate EPS (IEPS) stuff */
	memcpy(eps->inter_anchor_string, "_DMI_", 5);

	/* FIXME: implement vpd_table_length() and vpd_num_structures() */
	eps->table_length = structure_table_len;

#ifdef CONFIG_EPS_STRUCTURE_TABLE_ADDRESS
	/* FIXME: this needs to be better */
	eps->table_address = CONFIG_EPS_STRUCTURE_TABLE_ADDRESS;
#else
	/* immediately follow the entry point structure, assuming EPS is at
	   address 0x00000000 */
	eps->table_address = eps->entry_length;
#endif
#ifdef CONFIG_EPS_NUM_STRUCTURES
	eps->table_entry_count = CONFIG_EPS_NUM_STRUCTURES;
#else
	eps->table_entry_count = num_structures;
#endif
	eps->bcd_revision = (CONFIG_EPS_VPD_MAJOR_VERSION << 4) |
	                    CONFIG_EPS_VPD_MINOR_VERSION;

	/* calculate IEPS checksum first, then the EPS checksum */
	eps->inter_anchor_cksum = zero8_csum(&eps->inter_anchor_string[0], 0xf);
	eps->entry_cksum = zero8_csum((uint8_t *)eps, eps->entry_length);

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)eps->entry_length);
	print_buf(LOG_DEBUG, eps, eps->entry_length);

	return eps;
}

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
	header->type = 0;
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
	             strlen(CONFIG_SYSTEM_SKU_NUMBER) + 1 +
	             strlen(CONFIG_SYSTEM_FAMILY) + 1 +
		     1;			/* structure terminator */
	total_len = len + struct_len;

	*buf = realloc(*buf, total_len);
	memset(*buf + len, 0, struct_len);

	header = *buf + len;
	data = (uint8_t *)header + sizeof(*header);
	strings = (uint8_t *)data + sizeof(*data);

	/* fill in structure header details */
	header->type = 1;
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
	                 CONFIG_SYSTEM_SKU_NUMBER, '\0',
	                 CONFIG_SYSTEM_FAMILY, '\0');

	memset(*buf + struct_len, 0, 1);	/* terminator */

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)total_len);
	print_buf(LOG_DEBUG, data, total_len);

	return total_len;

vpd_create_type1_fail:
	return -1;
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

	struct_len = sizeof(struct vpd_table_eot) + 2;
	total_len = len + struct_len;
	*buf = realloc(*buf, total_len);

	data = *buf + len;
	data->header.type = 127;
	data->header.length = sizeof(*data);
	data->header.handle = handle;

	memset(*buf + struct_len, 0, 2);	/* double terminator */

	return total_len;
}

void vpd_free_table(void *data)
{
	uint8_t *foo = data;

	/* clean-up is trivially simple, for now... */
	free(foo);
}
