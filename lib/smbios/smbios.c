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
#include <sys/klog.h>

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "mosys/output.h"

#include "intf/mmio.h"

#include "lib/file.h"
#include "lib/smbios.h"
#include "lib/string.h"

/* Iterator used for table parsing */
struct smbios_iterator {
	struct smbios_entry *entry;	/* entry pointer */
	struct smbios_header *header;	/* current header */
	uint8_t *data;			/* table data */
	uint8_t *current;		/* pointer into data */
};

static struct smbios_iterator *smbios_itr = NULL;

/*
 * smbios_parse_string_table  -  parse strings into table structure
 *
 * @ptr:	pointer to start of strings
 * @strings:	array of SMBIOS_MAX_STRINGS SMBIOS_MAX_STRING_LENGTH-char
 *              strings to fill in.
 *
 * returns number of strings present in table, < 0 on error.
 */
int smbios_parse_string_table(char *ptr,
         char strings[SMBIOS_MAX_STRINGS][SMBIOS_MAX_STRING_LENGTH])
{
	int id, len, i;

	if (ptr == NULL || strings == NULL)
		return -1;

	/* The first string is purposefully nulled out because these fields
	 * directly indexed by SMBIOS string indexes (1-based). If a 0 is
	 * accidentaly used as an index make sure there is an empty string
	 * waiting. */
	strings[0][0] = '\0';

	for (id = 1; *ptr; ptr += len + 1) {
		len = strlen(ptr);
		/* Fill in string if there is room left. */
		if (id < SMBIOS_MAX_STRINGS) {
			/* filter non-ascii characters */
			for (i = 0; i < len &&
			     i < SMBIOS_MAX_STRING_LENGTH - 1; i++) {
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
 * smbios_string_table_len -  find length of smbios string table
 *
 * @ptr:	pointer to start of strings
 *
 * returns offset to end of string list (including terminating null byte).
 * returns < 0 if ptr is invalid.
 */
int smbios_string_table_len(const char *ptr)
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
 * smbios_get_string  -  return string at index
 *
 * @ptr:	pointer to start of strings
 * @num:	number of string to return (0-based)
 *
 * returns pointer to string
 */
char *smbios_get_string(const char *ptr, int num)
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
 * smbios_find_entry  -  locate entry pointer in EBDA
 *
 * @intf:	platform interface
 * @entry:	buffer to store entry pointer must be allocated by caller
 * @baseaddr:	address to begin search
 * @len:	length of region to search (in bytes)
 *
 * returns 0 if found
 * returns <0 if not found
 */
int smbios_find_entry(struct platform_intf *intf, struct smbios_entry *entry,
                      unsigned long int baseaddr, unsigned long int len)
{
	uint8_t csum;
	uint8_t *data;
	size_t offset;
	int i;
	uint8_t smbios_magic[] = SMBIOS_ENTRY_MAGIC;
	int found = 0;

	data = mmio_map(intf, O_RDONLY, baseaddr, len);
	if (data == NULL) {
		lprintf(LOG_DEBUG, "Unable to map SMBIOS entry buffer.\n");
		return -1;
	}

	if (find_pattern(data, len,
	                 &smbios_magic[0], 4, 16, &offset) < 0) {
		mmio_unmap(intf, data, baseaddr, len);
		found = 0;
	} else {
		found = 1;
	}

	if (!found) {
		char *buf;
		int klog_size;
		size_t klog_offset;

		/*
		 * Try parsing kernel log for SMBIOS= address. This is useful
		 * in situations where the SMBIOS address is out of spec and not
		 * easy to find from userspace (possibly due to EFI).
		 */
		lprintf(LOG_DEBUG, "%s: Attempting to get SMBIOS base address "
		                   "from kernel log\n", __func__);

		if ((klog_size = klogctl(10, NULL, 0)) < 0)
			return -1;

		buf = malloc(klog_size);
		if ((klogctl(3, buf, klog_size)) < 0) {
			lprintf(LOG_DEBUG, "Unable to read kernel log\n");
		} else {
			if (find_pattern(buf, klog_size,
			                 "SMBIOS=", 7, 1, &klog_offset) == 0) {
				baseaddr = strtoull(buf + klog_offset + 7,
				                    NULL, 0);
				lprintf(LOG_DEBUG, "kernel log offset: %d, "
				                   "SMBIOS=0x%08x\n",
					           klog_offset, baseaddr);
				data = mmio_map(intf, O_RDONLY, baseaddr, len);
				offset = 0;
				found = 1;
			} else {
				lprintf(LOG_DEBUG, "SMBIOS entry point not"
				                   "found in kernel log\n");
			}
		}

		free(buf);
	}

	/*
	 * Try obtaining SMBIOS address from /var/log/messages (first entry
	 * found will be used). This should be considered unreliable and used
	 * as a last resort.
	 */
	if (!found) {
		FILE *fp;
		char *line = NULL;
		size_t line_offset, bytes;

		lprintf(LOG_DEBUG, "%s: Attempting to get SMBIOS base address "
		                   "from /var/log/messages\n", __func__);
		fp = fopen("/var/log/messages", "r");
		/* read /var/log/messages one line at a time since it can get
		   rather large */
		while (fp && ((getline(&line, &bytes, fp)) > 0)) {
			if (find_pattern(line, bytes,
			             "SMBIOS=", 7, 1, &line_offset) == 0) {
				baseaddr = strtoull(line + line_offset + 7,
				                    NULL, 0);
				lprintf(LOG_DEBUG, "%s: SMBIOS=0x%08x\n",
					           __func__, baseaddr);
				data = mmio_map(intf, O_RDONLY, baseaddr, len);
				offset = 0;
				found = 1;
			}
			if (line) {
				free(line);
				line = NULL;
			}

			if (found)
				break;
		}
		fclose(fp);
	}

	if (!found) {
		lprintf(LOG_DEBUG, "Unable to find SMBIOS entry.\n");
		return -1;
	}


	lprintf(LOG_DEBUG, "SMBIOS Table Entry @ 0x%x\n", baseaddr + offset);

	/* copy entry into user-provided buffer */
	memset(entry, 0, sizeof(*entry));
	memcpy(entry, data + offset, sizeof(*entry));

	/* verify entry pointer checksum */
	for (csum = i = 0; i < entry->entry_length; i++)
		csum += data[offset + i];

	mmio_unmap(intf, data, baseaddr, len);

	if (csum != 0) {
		lprintf(LOG_DEBUG, "Invalid SMBIOS checksum: %02x\n", csum);
	}

	lprintf(LOG_DEBUG, "SMBIOS Entry Version:   %d.%d\n",
	        entry->major_ver, entry->minor_ver);
	lprintf(LOG_DEBUG, "SMBIOS Table Address:   0x%08x\n",
	        entry->table_address);
	lprintf(LOG_DEBUG, "SMBIOS Table Length:    %d\n",
	        entry->table_length);
	lprintf(LOG_DEBUG, "SMBIOS Table Count:     %d\n",
	        entry->table_entry_count);

	return 0;
}

/*
 * smbios_itr_destroy  -  clean up iterator
 *
 * @intf:	platform interface
 */
static void smbios_itr_destroy(void *arg)
{
	struct platform_intf *intf = arg;
	if (smbios_itr) {
		/* clean up smbios table buffer */
		if (smbios_itr->data) {
			mmio_unmap(intf, smbios_itr->data,
			           smbios_itr->entry->table_address,
			           smbios_itr->entry->table_length);
		}
		/* clean up smbios entry */
		if (smbios_itr->entry)
			free(smbios_itr->entry);
		/* clean up iterator */
		free(smbios_itr);
		smbios_itr = NULL;
	}
}

/*
 * smbios_itr_setup  -  setup iterator for smbios searching
 *
 * @intf:	platform interface
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns 0 to indicate successful setup or reset to setup
 * returns <0 to indicate failure
 */
static int smbios_itr_setup(struct platform_intf *intf,
                            unsigned int baseaddr, unsigned int len)
{
	/* already setup? */
	if (smbios_itr) {
		/* reset to start of tables */
		smbios_itr->current = smbios_itr->data;
		smbios_itr->header =
		        (struct smbios_header *)smbios_itr->current;
		return 0;
	}

	/* setup iterator */
	smbios_itr = mosys_malloc(sizeof(*smbios_itr));
	memset(smbios_itr, 0, sizeof(*smbios_itr));

	smbios_itr->entry = mosys_malloc(sizeof(*smbios_itr->entry));
	memset(smbios_itr->entry, 0, sizeof(*smbios_itr->entry));

	/* search for entry pointer */
	if (smbios_find_entry(intf, smbios_itr->entry, baseaddr, len) < 0) {
		smbios_itr_destroy(intf);
		return -1;
	}
	if (smbios_itr->entry->table_length == 0) {
		smbios_itr_destroy(intf);
		return -1;
	}

	/* mmap in entire smbios area */
	smbios_itr->data = mmio_map(intf, O_RDONLY,
	                            smbios_itr->entry->table_address,
	                            smbios_itr->entry->table_length);

	if (smbios_itr->data == NULL) {
		lprintf(LOG_ERR, "Unable to find SMBIOS tables at 0x%08x\n",
		        smbios_itr->entry->table_address);
		smbios_itr_destroy(intf);
		return -1;
	}

	if (mosys_get_verbosity() == LOG_DEBUG)
		print_buffer(smbios_itr->data, smbios_itr->entry->table_length);

	/* start pointer at table start */
	smbios_itr->current = smbios_itr->data;
	smbios_itr->header = (struct smbios_header *)smbios_itr->current;

	/* make sure we get torn down at exit time. */
	add_destroy_callback(smbios_itr_destroy, intf);

	return 0;
}

/*
 * smbios_itr_next  -  find next table from current iterator
 *
 * @intf	platform interface
 *
 * returns 0 if found
 * returns 1 if search wrapped to start
 * returns <0 if iterator not setup or invalid
 */
static int smbios_itr_next(struct platform_intf *intf)
{
	int rc = 0;

	if (!smbios_itr || !smbios_itr->current || !smbios_itr->header)
		return -1;

	/* adjust pointer to end of table data + strings */
	smbios_itr->current += smbios_itr->header->length +
		smbios_string_table_len((char *)smbios_itr->current +
		                        smbios_itr->header->length);

	/* check for overflow */
	if (smbios_itr->current >= (smbios_itr->data +
				    smbios_itr->entry->table_length - 1)) {
		/* start pointer over */
		lprintf(LOG_DEBUG, "smbios_itr_next: search wrapped\n");
		smbios_itr->current = smbios_itr->data;
		rc = 1;
	}

	/* adjust header pointer */
	smbios_itr->header = (struct smbios_header *)smbios_itr->current;

	return rc;
}

/*
 * smbios_find_table_raw  -  locate table and store in buffer
 *
 * @intf:	platform interface
 * @type:	smbios table type
 * @instance:	smbios table instance
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns 0 if found
 * returns <0 if not found
 */
static int smbios_find_table_raw(struct platform_intf *intf,
                                 enum smbios_types type, int instance,
                                 unsigned int baseaddr, unsigned int len)
{
	int ret = -1;

	/* start iterator */
	if (smbios_itr_setup(intf, baseaddr, len) >= 0) {
		do {
			/* check for correct type */
			if (smbios_itr->header->type != type)
				continue;

			/* check for correct instance */
			if (instance-- > 0)
				continue;

			if (mosys_get_verbosity() == LOG_DEBUG)
				print_buffer(smbios_itr->current,
				             smbios_itr->header->length);

			/* found */
			ret = 0;
			break;

		} while (smbios_itr_next(intf) == 0);
	}

	return ret;
}

/*
 * smbios_find_table  -  locate specified SMBIOS table in memory
 *
 * @intf:	platform interface
 * @type:	smbios table type to locate
 * @instance:	what instance to retrieve (0-based)
 * @table:	OUTPUT buffer to store table data
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int smbios_find_table(struct platform_intf *intf, enum smbios_types type,
                      int instance, struct smbios_table *table,
                      unsigned int baseaddr, unsigned int len)
{
	if (!table || type > SMBIOS_TYPE_END)
		return -1;

	/* get the table as raw buffer */
	if (smbios_find_table_raw(intf, type, instance, baseaddr, len) < 0) {
		lprintf(LOG_DEBUG, "Unable to locate table %d:%d\n",
		        type, instance);
		return -1;
	}

	/* copy header first */
	memset(table, 0, sizeof(*table));
	memcpy(&table->header, smbios_itr->header, sizeof(table->header));

	/* then table data */
	memcpy(&table->data.data, smbios_itr->current + sizeof(table->header),
	       smbios_itr->header->length - sizeof(table->header));

	/* finally parse table strings */
	smbios_parse_string_table((char *)smbios_itr->current +
	                          smbios_itr->header->length, table->string);

	return 0;
}

/*
 * smbios_find_string  -  locate specific string in SMBIOS table
 *                        (conforms to legacy smbios utility)
 *
 * @intf:	platform interface
 * @type:	smbios table type
 * @number:	string number to retrieve
 * @baseaddr:	base address to start searching in
 * @len:	length of region to search (in bytes)
 *
 * returns allocated buffer containing null-terminated string
 *         !! caller is expected to free returned buffer !!
 * returns NULL to indicate error
 */
char *smbios_find_string(struct platform_intf *intf,
                         enum smbios_types type, int number,
                         unsigned int baseaddr, unsigned int len)
{
	char *sptr;

	if (type > SMBIOS_TYPE_END)
		return NULL;

	/* get instance 0 of the table */
	if (smbios_find_table_raw(intf, type, 0, baseaddr, len) < 0) {
		lprintf(LOG_DEBUG, "Unable to locate table %d\n", type);
		return NULL;
	}

	/* lookup string location in table */
	sptr = smbios_get_string((char *)smbios_itr->current +
				 smbios_itr->header->length,
				 number);

	if (!sptr) {
		lprintf(LOG_DEBUG, "String %d not found in table %d\n",
		        number, type);
		return NULL;
	}

	/* return allocated copy of string */
	return mosys_strdup(sptr);
}
