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

#include <errno.h>
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
 * vpd_append_type0 - append type 0 (firmware info) structure to a buffer
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 * @vendor:	firmware vendor (string)
 * @version:	firmware version (string)
 * @start:	offset of firmware runtime code in ROM image
 * @date:	firmware release date (string)
 * @rom_size:	ROM size (in 64KB blocks)
 * @major_ver:	firmware major version
 * @minor_ver:	firmware minor version
 * @ec_major_ver: EC firmware major version
 * @ec_minor_ver: EC firmware minor version
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
int vpd_append_type0(uint16_t handle, uint8_t **buf, size_t len,
                     char *vendor, char *version, uint16_t start, char *date,
                     uint8_t rom_size, uint8_t major_ver, uint8_t minor_ver,
		     uint8_t ec_major_ver, uint8_t ec_minor_ver)
{
	struct vpd_header *header;
	struct vpd_table_firmware *data;
	uint8_t *strings, *p;
	size_t struct_len, total_len;

	/* FIXME: Add sanity checking */
	struct_len = sizeof(struct vpd_header) +
	             sizeof(struct vpd_table_firmware) +
	             strlen(vendor) + 1 +
	             strlen(version) + 1 +
	             strlen(date) + 1 +
		     1;			/* structure terminator */
	total_len = len + struct_len;

	*buf = realloc(*buf, total_len);
	memset(*buf + len, 0, struct_len);

	p = *buf + len;
	header = (struct vpd_header *)p;
	p += sizeof(*header);
	data = (struct vpd_table_firmware *)p;
	p += sizeof(*data);
	strings = p;

	/* fill in structure header details */
	header->type = VPD_TYPE_FIRMWARE;
	header->length = sizeof(*header) + sizeof(*data);
	header->handle = handle;

	data->vendor = 1;
	data->version = 2;
	data->start_address = start,
	data->rom_size_64k_blocks = rom_size - 1;
	data->major_ver = major_ver;
	data->minor_ver = minor_ver;
	data->ec_major_ver = ec_major_ver;
	data->ec_minor_ver = ec_minor_ver;

	data->release_date = 3;
	sprintf(strings, "%s%c%s%c%s%c",
	                 vendor, '\0',
	                 version, '\0',
	                 date, '\0');

	memset(&buf[struct_len], 0, 1);	/* terminator */

	lprintf(LOG_DEBUG, "%s: total length (including strings): %u\n",
	        __func__, (unsigned)total_len);
	print_buf(LOG_DEBUG, data, total_len);
	return total_len;
}

/**
 * sym2type0 - extract symbols and append firmware info table to buffer
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
int sym2type0(uint16_t handle, uint8_t **buf, size_t len)
{
	char *vendor, *version, *release_date;
	uint16_t start;
	uint8_t rom_size, major_ver, minor_ver, ec_major_ver, ec_minor_ver;
	char *tmp;

	errno = 0;
	vendor = version = release_date = NULL;

	/* strings */
	if (!(vendor = sym2str("CONFIG_FIRMWARE_VENDOR"))) return -1;
	if (!(version = sym2str("CONFIG_FIRMWARE_VERSION"))) return -1;
	if (!(release_date = sym2str("CONFIG_FIRMWARE_RELEASE_DATE"))) return -1;

	/* optional values */
	tmp = sym2str("CONFIG_FIRMWARE_START");
	if (tmp) {
		start = (uint16_t)strtoul(tmp, NULL, 0);
		if (errno) return -1;
	} else {
		/* set default value */
		start = 0;
	}

	/* required values */
	if (!(tmp = sym2str("CONFIG_FIRMWARE_ROM_SIZE"))) return -1;
	rom_size = (uint8_t)strtoul(tmp, NULL, 0);
	if (errno) return -1;

	if (!(tmp = sym2str("CONFIG_SYSTEM_FIRMWARE_MAJOR_RELEASE"))) return -1;
	major_ver = (uint8_t)strtoul(tmp, NULL, 0);
	if (errno) return -1;

	if (!(tmp = sym2str("CONFIG_SYSTEM_FIRMWARE_MINOR_RELEASE"))) return -1;
	minor_ver = (uint8_t)strtoul(tmp, NULL, 0);
	if (errno) return -1;

	if (!(tmp = sym2str("CONFIG_EC_FIRMWARE_MAJOR_RELEASE"))) return -1;
	ec_major_ver = (uint8_t)strtoul(tmp, NULL, 0);
	if (errno) return -1;

	if (!(tmp = sym2str("CONFIG_EC_FIRMWARE_MINOR_RELEASE"))) return -1;
	lprintf(LOG_INFO, "checkpoint 7\n");
	ec_minor_ver = (uint8_t)strtoul(tmp, NULL, 0);
	lprintf(LOG_INFO, "checkpoint 8\n");
	if (errno) return -1;

	return vpd_append_type0(handle, buf, len, vendor, version, start,
	                        release_date, rom_size, major_ver, minor_ver,
				ec_major_ver, ec_minor_ver);
}
