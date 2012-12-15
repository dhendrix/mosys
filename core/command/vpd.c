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

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "mosys/log.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"

#include "lib/string.h"
#include "lib/vpd.h"
#include "lib/vpd_tables.h"

static int vpd_find_string_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd, int argc, char **argv)
{
	uint8_t type, num;
	char *str;
	int rc = 0;

	/* this is legacy tool format */
	if (argc < 2) {
		platform_cmd_usage(cmd);
		return -1;
	}

	type = (uint8_t) strtoul(argv[0], NULL, 0);
	num = (uint8_t) strtoul(argv[1], NULL, 0);

	lprintf(LOG_DEBUG, "vpd get %d %d\n", type, num);

	/* table offset starts at 0, string offset starts at 1 */
	if (num == 0)
		return 0;	/* legacy tool always returns success */
	num--;

	/* locate and display specified string */
	str = vpd_find_string(intf, type, num, vpd_rom_base, vpd_rom_size);
	if (str) {
		struct kv_pair *kv = kv_pair_new();
		kv_pair_fmt(kv, "string", str);
		rc = kv_pair_print(kv);
		kv_pair_free(kv);
		free(str);
	}

	return rc;		/* the legacy tool always returns success */
}

static int vpd_find_blob_cmd(struct platform_intf *intf,
                             struct platform_cmd *cmd, int argc, char **argv)
{
	return -1;
}

static int vpd_print_firmware_cmd(struct platform_intf *intf,
                                  struct platform_cmd *cmd, int argc, char **argv)
{
	struct vpd_table table;
	struct kv_pair *kv;
	int rc;

	if (vpd_find_table(intf, VPD_TYPE_FIRMWARE, 0, &table,
	                   vpd_rom_base, vpd_rom_size) < 0)
		return 0;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "table_type", "%d", table.header.type);
	kv_pair_add(kv, "vendor", table.string[table.data.firmware.vendor]);
	kv_pair_add(kv, "version", table.string[table.data.firmware.version]);
	kv_pair_add(kv, "release_date",
		    table.string[table.data.firmware.release_date]);
	kv_pair_fmt(kv, "size", "%u KB",
		    (table.data.firmware.rom_size_64k_blocks + 1) * 64);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int vpd_print_system_cmd(struct platform_intf *intf,
                                struct platform_cmd *cmd,
                                int argc, char **argv)
{
	struct vpd_table table;
	struct kv_pair *kv;
	int rc;

	if (vpd_find_table(intf, VPD_TYPE_SYSTEM, 0, &table,
	                   vpd_rom_base, vpd_rom_size) < 0)
		return 0;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "table_type", "%d", table.header.type);
	kv_pair_add(kv, "manufacturer",
		    table.string[table.data.system.manufacturer]);
	kv_pair_add(kv, "name", table.string[table.data.system.name]);
	kv_pair_add(kv, "version", table.string[table.data.system.version]);
	kv_pair_add(kv, "serial_number",
		    table.string[table.data.system.serial_number]);
	kv_pair_add(kv, "sku",
		    table.string[table.data.system.sku_number]);
	kv_pair_add(kv, "family",
		    table.string[table.data.system.family]);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int vpd_print_blobs_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	struct vpd_table table;
	struct kv_pair *kv, *blob_kv;
	int i, rc;

	for (i = 0; i < 0xffff; i++) {
		if (vpd_find_table(intf, VPD_TYPE_BINARY_BLOB_POINTER,
		                   i, &table, vpd_rom_base, vpd_rom_size) < 0) {
			lprintf(LOG_DEBUG, "cannot find binary blob pointer\n");
			break;
		}

		kv = kv_pair_new();
		blob_kv = kv_pair_new();

		kv_pair_fmt(kv, "table_type", "%d", table.header.type);
		kv_pair_fmt(kv, "handle", "%u", table.header.handle);
		kv_pair_add(kv, "vendor",
			    table.string[table.data.blob.vendor]);
		kv_pair_add(kv, "description",
		            table.string[table.data.blob.description]);
		kv_pair_fmt(kv, "offset", "0x%08x", table.data.blob.offset);
		kv_pair_fmt(kv, "size", "%d", table.data.blob.size);

		vpd_print_blob(intf, blob_kv, &table);

		rc = kv_pair_print(kv);
		kv_pair_free(kv);
		if (rc)
			break;

		rc = kv_pair_print(blob_kv);
		kv_pair_free(blob_kv);
		if (rc)
			break;
	}

	return rc;
}

static int vpd_print_all_cmd(struct platform_intf *intf,
                             struct platform_cmd *cmd,
                             int argc, char **argv)
{
	int rc = 0;

	rc |= vpd_print_firmware_cmd(intf, cmd, argc, argv);
	rc |= vpd_print_system_cmd(intf, cmd, argc, argv);
	rc |= vpd_print_blobs_cmd(intf, cmd, argc, argv);

	return rc;
}

static int vpd_dump_blobs_cmd(struct platform_intf *intf,
                              struct platform_cmd *cmd,
                              int argc, char **argv)
{
	int handle, type, start, end;

	if ((argc < 1)) {
		platform_cmd_usage(cmd);
		return -1;
	}
	type = strtol(argv[0], NULL, 0);
	if (type != VPD_TYPE_BINARY_BLOB_POINTER)
		return -1;

	if (argc == 2) {
		handle = strtol(argv[1], NULL, 0);
		if ((handle < 0) || (handle > 0xffff) || (errno == ERANGE)) {
			lprintf(LOG_ERR, "bad handle: %s\n", argv[1]);
			return -1;
		}
		start = handle;
		end = start + 1;
	} else {
		start = 0;
		end = 0xffff;
	}

	/* find the VPD structure table entry */
	for (handle = start; handle < end; handle++) {
		int size, fd = -1;
		uint8_t *buf;
		struct vpd_table table;
		char filename[128];	/* FIXME: use string_builder here */

		if (vpd_find_table(intf, type, handle, &table,
		                   vpd_rom_base, vpd_rom_size) < 0) {
			lprintf(LOG_DEBUG, "cannot find binary blob pointer\n");
			break;
		}

		size = vpd_get_blob(intf, &table.data.blob, &buf);
		sprintf(filename, "%s%d_%d.bin", "type", type, handle);

		if ((fd = open(filename,
		               O_CREAT | O_WRONLY | O_TRUNC,
			       S_IRUSR | S_IWUSR | S_IRGRP)) < 0) {
			lperror(LOG_ERR, "Could not create file \"%s\": %s",
			        filename, strerror(errno));
			free(buf);
			return -1;
		}

		if (write(fd, buf, size) != size) {
			lperror(LOG_ERR, "Could not write file \"%s\": %s",
			        filename, strerror(errno));
			free(buf);
			close(fd);
			return -1;
		}

		lprintf(LOG_NOTICE, "Wrote %s\n", filename);
		close(fd);
		free(buf);
	}

	return 0;
}

struct platform_cmd vpd_find_cmds[] = {
	{
		.name	= "string",
		.desc	= "Retrieve a Specified String",
		.usage	= "<table type> <string number>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_find_string_cmd }
	},
	{
		.name	= "blob",
		.desc	= "Decode Specified Binary Blob",
		.usage	= "<vendor> <description>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_find_blob_cmd }
	},
	{ NULL },
};

struct platform_cmd vpd_print_cmds[] = {
	{
		.name	= "all",
		.desc	= "Print All Tables",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_print_all_cmd }
	},
	{
		.name	= "firmware",
		.desc	= "Firmware Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_print_firmware_cmd }
	},
	{
		.name	= "system",
		.desc	= "System Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_print_system_cmd }
	},
	{
		.name	= "blobs",
		.desc	= "Binary Blobs Pointer Tables",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_print_blobs_cmd }
	},
	{ NULL }
};

struct platform_cmd vpd_dump_cmds[] = {
#if 0
	{
		.name	= "all",
		.desc	= "Print All Tables",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_dump_all_cmd }
	},
	{
		.name	= "firmware",
		.desc	= "Firmware Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_dump_firmware_cmd }
	},
	{
		.name	= "system",
		.desc	= "System Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_dump_system_cmd }
	},
#endif
	{
		.name	= "blobs",
		.desc	= "Binary Blobs",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_dump_blobs_cmd }
	},
	{ NULL }
};

struct platform_cmd vpd_cmds[] = {
	{
		.name	= "print",
		.desc	= "Print Known VPD Tables",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = vpd_print_cmds }
	},
	{
		.name	= "dump",
		.desc	= "Dump VPD Structure to File",
		.usage	= "<type>",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = vpd_dump_cmds }
	},
	{
		.name	= "find",
		.desc	= "Find Specific Information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = vpd_find_cmds }
	},
	{ NULL }
};

struct platform_cmd cmd_vpd = {
	.type	= ARG_TYPE_SUB,
	.name	= "vpd",
	.desc	= "Vital Product Data",
	.arg	= { .sub = vpd_cmds },
};
