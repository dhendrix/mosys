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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"

#include "lib/string.h"
#include "lib/vpd.h"
#include "lib/vpd_tables.h"

static int vpd_dump_cmd(struct platform_intf *intf,
                        struct platform_cmd *cmd, int argc, char **argv)
{
	/* FIXME: implement this */
	return -ENOSYS;
}

static int vpd_find_string_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd, int argc, char **argv)
{
	uint8_t type, num;
	char *str;

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
		kv_pair_print(kv);
		kv_pair_free(kv);
		free(str);
	}

	return 0;		/* the legacy tool always returns success */
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

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int vpd_print_system_cmd(struct platform_intf *intf,
                                struct platform_cmd *cmd,
                                int argc, char **argv)
{
	struct vpd_table table;
	struct kv_pair *kv;

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

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int vpd_print_blobs_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	struct vpd_table table;
	struct kv_pair *kv, *blob_kv;
	int i;

	for (i = 0; i < 0xffff; i++) {
		if (vpd_find_table(intf, VPD_TYPE_BINARY_BLOB_POINTER,
		                   i, &table, vpd_rom_base, vpd_rom_size) < 0) {
			lprintf(LOG_DEBUG, "cannot find binary blob pointer\n");
			break;
		}

		kv = kv_pair_new();
		blob_kv = kv_pair_new();

		kv_pair_fmt(kv, "table_type", "%d", table.header.type);
		kv_pair_add(kv, "vendor",
			    table.string[table.data.blob.vendor]);
		kv_pair_add(kv, "description",
		            table.string[table.data.blob.description]);
		kv_pair_fmt(kv, "offset", "0x%08x", table.data.blob.offset);
		kv_pair_fmt(kv, "size", "%d", table.data.blob.size);

		vpd_print_blob(intf, blob_kv, &table);

		kv_pair_print(kv);
		kv_pair_free(kv);
		kv_pair_print(blob_kv);
		kv_pair_free(blob_kv);
	}

	return 0;

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

struct platform_cmd vpd_cmds[] = {
	{
		.name	= "print",
		.desc	= "Print Known VPD Tables",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = vpd_print_cmds }
	},
	{
		.name	= "dump",
		.desc	= "Dump VPD to File (does not include binary blobs)",
		.usage	= "<file>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = vpd_dump_cmd }
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
