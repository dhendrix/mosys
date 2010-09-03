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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <uuid/uuid.h>

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "mosys/output.h"

#include "intf/mmio.h"

#include "lib/string.h"
#include "lib/vpd.h"
#include "lib/vpd_binary_blob.h"

/*
 * These represent the memory-mapped address space which the VPD
 * can be found in. That is, the VPD resides somewhere in the range
 * vpd_rom_base to (vpd_rom_base + vpd_rom_len).
 */
unsigned int vpd_rom_base;
unsigned int vpd_rom_size;

/* Iterator used for table parsing */
struct vpd_iterator {
	struct vpd_entry *entry;	/* entry pointer */
	struct vpd_header *header;	/* current header */
	uint8_t *data;			/* table data */
	uint8_t *current;		/* pointer into data */
};

static struct vpd_iterator *vpd_itr = NULL;

/*
 * vpd_parse_string_table  -  parse strings into table structure
 *
 * @ptr:	pointer to start of strings
 * @strings:	array of VPD_MAX_STRINGS VPD_MAX_STRING_LENGTH-char
 *              strings to fill in.
 *
 * returns number of strings present in table, < 0 on error.
 */
int vpd_parse_string_table(char *ptr,
         char strings[VPD_MAX_STRINGS][VPD_MAX_STRING_LENGTH])
{
	int id, len, i;

	if (ptr == NULL || strings == NULL)
		return -1;

	/* The first string is purposefully nulled out because these fields
	 * directly indexed by VPD string indexes (1-based). If a 0 is
	 * accidentaly used as an index make sure there is an empty string
	 * waiting. */
	strings[0][0] = '\0';

	for (id = 1; *ptr; ptr += len + 1) {
		len = strlen(ptr);
		/* Fill in string if there is room left. */
		if (id < VPD_MAX_STRINGS) {
			/* filter non-ascii characters */
			for (i = 0; i < len &&
			     i < VPD_MAX_STRING_LENGTH - 1; i++) {
				if (isprint(ptr[i]))
					strings[id][i] = ptr[i];
				else
					strings[id][i] = '.';
			}
			strings[id][i] = '\0';
		}
		++id;
	}

	return id - 1;
}

/*
 * vpd_string_table_len -  find length of vpd string table
 *
 * @ptr:	pointer to start of strings
 *
 * returns offset to end of string list (including terminating null byte).
 * returns < 0 if ptr is invalid.
 */
int vpd_string_table_len(const char *ptr)
{
	int len, tlen;

	if (ptr == NULL)
		return -1;

	/* empty string list */
	if (!*ptr)
		return 2;

	/* count each string length */
	for (tlen = 1; ptr && *ptr; ptr += len) {
		len = strlen(ptr) + 1;
		tlen += len;
	}

	return tlen;
}

/*
 * vpd_get_string  -  return string at index
 *
 * @ptr:	pointer to start of strings
 * @num:	number of string to return (0-based)
 *
 * returns pointer to string
 */
char *vpd_get_string(const char *ptr, int num)
{
	if (num < 0 || ptr == NULL)
		return NULL;

	for (; num && ptr && *ptr; num--)
		ptr += strlen(ptr) + 1;

	if (!ptr || !*ptr)
		return NULL;
	else
		return (char *)ptr;
}

/*
 * vpd_find_entry  -  locate entry pointer
 *
 * @intf:	platform interface
 * @entry:	buffer to store entry pointer must be allocated by caller
 * @baseaddr:	address to begin search
 * @len:	length of region to search (in bytes)
 *
 * returns 0 if found
 * returns <0 if not found
 */
int vpd_find_entry(struct platform_intf *intf, struct vpd_entry *entry,
                   unsigned long int baseaddr, unsigned long int len)
{
	uint8_t csum;
	uint8_t *data;
	size_t offset;
	int i;
	uint8_t vpd_magic[] = VPD_ENTRY_MAGIC;

	data = mmio_map(intf, O_RDONLY, baseaddr, len);
	if (data == NULL) {
		lprintf(LOG_DEBUG, "Unable to map VPD entry buffer.\n");
		return -1;
	}

	if (find_pattern(data, len,
	                 &vpd_magic[0], 4, 16, &offset) < 0) {
		lprintf(LOG_DEBUG, "Unable to find VPD entry.\n");
		mmio_unmap(intf, data, baseaddr, len);
		return -1;
	}

	lprintf(LOG_DEBUG, "VPD Table Entry @ 0x%x\n", baseaddr + offset);

	/* copy entry into user-provided buffer */
	memset(entry, 0, sizeof(*entry));
	memcpy(entry, data + offset, sizeof(*entry));

	/* verify entry pointer checksum */
	for (csum = i = 0; i < entry->entry_length; i++)
		csum += data[offset + i];

	mmio_unmap(intf, data, baseaddr, len);

	if (csum != 0) {
		lprintf(LOG_DEBUG, "Invalid VPD checksum: %02x\n", csum);
	}

	lprintf(LOG_DEBUG, "VPD Entry Version:   %d.%d\n",
	        entry->major_ver, entry->minor_ver);
	lprintf(LOG_DEBUG, "VPD Table Offset:    0x%08x\n",
	        entry->table_address);
	lprintf(LOG_DEBUG, "VPD Table Length:    %d\n",
	        entry->table_length);
	lprintf(LOG_DEBUG, "VPD Table Count:     %d\n",
	        entry->table_entry_count);

	return 0;
}

/*
 * vpd_itr_destroy  -  clean up iterator
 *
 * @intf:	platform interface
 */
static void vpd_itr_destroy(void *arg)
{
	struct platform_intf *intf = arg;
	if (vpd_itr) {
		/* clean up vpd table buffer */
		if (vpd_itr->data) {
			mmio_unmap(intf, vpd_itr->data,
			           vpd_itr->entry->table_address,
			           vpd_itr->entry->table_length);
		}
		/* clean up vpd entry */
		if (vpd_itr->entry)
			free(vpd_itr->entry);
		/* clean up iterator */
		free(vpd_itr);
		vpd_itr = NULL;
	}
}

/*
 * vpd_itr_setup  -  setup iterator for vpd searching
 *
 * @intf:	platform interface
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns 0 to indicate successful setup or reset to setup
 * returns <0 to indicate failure
 */
static int vpd_itr_setup(struct platform_intf *intf,
                         unsigned int baseaddr, unsigned int len)
{
	/* already setup? */
	if (vpd_itr) {
		/* reset to start of tables */
		vpd_itr->current = vpd_itr->data;
		vpd_itr->header =
		        (struct vpd_header *)vpd_itr->current;
		return 0;
	}

	/* setup iterator */
	vpd_itr = mosys_malloc(sizeof(*vpd_itr));
	memset(vpd_itr, 0, sizeof(*vpd_itr));

	vpd_itr->entry = mosys_malloc(sizeof(*vpd_itr->entry));
	memset(vpd_itr->entry, 0, sizeof(*vpd_itr->entry));

	/* search for entry pointer */
	if (vpd_find_entry(intf, vpd_itr->entry, baseaddr, len) < 0) {
		vpd_itr_destroy(intf);
		return -1;
	}
	if (vpd_itr->entry->table_length == 0) {
		vpd_itr_destroy(intf);
		return -1;
	}

	/* mmap in entire vpd area */
	vpd_itr->data = mmio_map(intf, O_RDONLY,
	                         baseaddr + vpd_itr->entry->table_address,
	                         vpd_itr->entry->table_length);

	if (vpd_itr->data == NULL) {
		lprintf(LOG_ERR, "Unable to find VPD tables at 0x%08x\n",
		        vpd_itr->entry->table_address);
		vpd_itr_destroy(intf);
		return -1;
	}

	if (mosys_get_verbosity() == LOG_DEBUG)
		print_buffer(vpd_itr->data, vpd_itr->entry->table_length);

	/* start pointer at table start */
	vpd_itr->current = vpd_itr->data;
	vpd_itr->header = (struct vpd_header *)vpd_itr->current;

	/* make sure we get torn down at exit time. */
	add_destroy_callback(vpd_itr_destroy, intf);

	return 0;
}

/*
 * vpd_itr_next  -  find next table from current iterator
 *
 * @intf	platform interface
 *
 * returns 0 if found
 * returns 1 if search wrapped to start
 * returns <0 if iterator not setup or invalid
 */
static int vpd_itr_next(struct platform_intf *intf)
{
	int rc = 0;

	if (!vpd_itr || !vpd_itr->current || !vpd_itr->header)
		return -1;

	/* adjust pointer to end of table data + strings */
	vpd_itr->current += vpd_itr->header->length +
		vpd_string_table_len((char *)vpd_itr->current +
		                        vpd_itr->header->length);

	/* check for overflow */
	if (vpd_itr->current >= (vpd_itr->data +
				    vpd_itr->entry->table_length)) {
		/* start pointer over */
		lprintf(LOG_DEBUG, "vpd_itr_next: search wrapped\n");
		vpd_itr->current = vpd_itr->data;
		rc = 1;
	}

	/* adjust header pointer */
	vpd_itr->header = (struct vpd_header *)vpd_itr->current;

	return rc;
}

/*
 * vpd_find_table_raw  -  locate table and store in buffer
 *
 * @intf:	platform interface
 * @type:	vpd table type
 * @instance:	vpd table instance
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns 0 if found
 * returns <0 if not found
 */
static int vpd_find_table_raw(struct platform_intf *intf,
                                 enum vpd_types type, int instance,
                                 unsigned int baseaddr, unsigned int len)
{
	int ret = -1;

	/* start iterator */
	if (vpd_itr_setup(intf, baseaddr, len) >= 0) {
		do {
			/* check for correct type */
			if (vpd_itr->header->type != type)
				continue;

			/* check for correct instance */
			if (instance-- > 0)
				continue;

			if (mosys_get_verbosity() == LOG_DEBUG)
				print_buffer(vpd_itr->current,
				             vpd_itr->header->length);

			/* found */
			ret = 0;
			break;

		} while (vpd_itr_next(intf) == 0);
	}

	return ret;
}

/*
 * vpd_find_table  -  locate specified VPD table in memory
 *
 * @intf:	platform interface
 * @type:	vpd table type to locate
 * @instance:	what instance to retrieve (0-based)
 * @table:	OUTPUT buffer to store table data
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int vpd_find_table(struct platform_intf *intf, enum vpd_types type,
                      int instance, struct vpd_table *table,
                      unsigned int baseaddr, unsigned int len)
{
	if (!table || type > 256)
		return -1;

	/* get the table as raw buffer */
	if (vpd_find_table_raw(intf, type, instance, baseaddr, len) < 0) {
		lprintf(LOG_DEBUG, "Unable to locate table %d:%d\n",
		        type, instance);
		return -1;
	}

	/* copy header first */
	memset(table, 0, sizeof(*table));
	memcpy(&table->header, vpd_itr->header, sizeof(table->header));

	/* then table data */
	memcpy(&table->data.data, vpd_itr->current + sizeof(table->header),
	       vpd_itr->header->length - sizeof(table->header));

	/* finally parse table strings */
	vpd_parse_string_table((char *)vpd_itr->current +
	                          vpd_itr->header->length, table->string);

	return 0;
}

/*
 * vpd_find_string  -  locate specific string in VPD table
 *
 * @intf:	platform interface
 * @type:	vpd table type
 * @number:	string number to retrieve
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns allocated buffer containing null-terminated string
 *         !! caller is expected to free returned buffer !!
 * returns NULL to indicate error
 */
char *vpd_find_string(struct platform_intf *intf,
                         enum vpd_types type, int number,
                         unsigned int baseaddr, unsigned int len)
{
	char *sptr;

	if (type > VPD_TYPE_END)
		return NULL;

	/* get instance 0 of the table */
	if (vpd_find_table_raw(intf, type, 0, baseaddr, len) < 0) {
		lprintf(LOG_ERR, "Unable to locate table %d\n", type);
		return NULL;
	}

	/* lookup string location in table */
	sptr = vpd_get_string((char *)vpd_itr->current +
				 vpd_itr->header->length,
				 number);

	if (!sptr) {
		lprintf(LOG_DEBUG, "String %d not found in table %d\n",
		        number, type);
		return NULL;
	}

	/* return allocated copy of string */
	return mosys_strdup(sptr);
}

struct blob_handler blob_handlers[] = {
	{ "b9468091-ccd2-4b8b-8300-8cd8336a14f6", print_agz_blob_v3 },
	{ "0eea3385-602a-4f6d-b593-6118dda238b2", print_agz_blob_v5 },
	{ NULL },
};

extern int vpd_print_blob(struct platform_intf *intf,
                          struct kv_pair *kv, struct vpd_table *table)
{
	struct blob_handler *handler;
	char s[37];
	int rc = 0;

	uuid_unparse(table->data.blob.uuid, s);

	for (handler = &blob_handlers[0]; handler && handler->print; handler++) {
		if (!strcmp(handler->uuid, s)) {
			lprintf(LOG_DEBUG, "found matching uuid\n");
			break;
		}
	}

	if (handler && handler->print) {
		uint8_t *blob = mosys_malloc(table->data.blob.size);
		
		if (mmio_read(intf, vpd_rom_base + table->data.blob.offset,
		              table->data.blob.size, blob) < 0) {
			lprintf(LOG_DEBUG, "%s: cannot map %lu bytes at offset "
					   "%lu\n", table->data.blob.offset,
					   table->data.blob.size);
			rc = -1;
		} else {
			lprintf(LOG_DEBUG, "%s: blob offset: 0x%x, blob size: "
			                   "%u\n", __func__,
			                   table->data.blob.offset,
			                   table->data.blob.size);

			rc = handler->print(blob, table->data.blob.size, kv);
		}

		free(blob);
	} else {
		lprintf(LOG_DEBUG, "%s: no suitable handler found\n");
	}

	return rc;
}
