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

#include "mosys/log.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"
#include "mosys/string.h"

#include "lib/smbios.h"
#include "lib/smbios_tables.h"

static int smbios_get_cmd(struct platform_intf *intf,
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

	lprintf(LOG_DEBUG, "smbios get %d %d\n", type, num);

	/* table offset starts at 0, string offset starts at 1 */
	if (num == 0)
		return 0;	/* legacy tool always returns success */
	num--;

	/* locate and display specified string */
	str = smbios_find_string(intf, type, num,
	                         SMBIOS_LEGACY_ENTRY_BASE,
	                         SMBIOS_LEGACY_ENTRY_LEN);
	if (str) {
		struct kv_pair *kv = kv_pair_new();
		kv_pair_fmt(kv, "string", str);
		kv_pair_print(kv);
		kv_pair_free(kv);
		free(str);
	} else {
		lprintf(LOG_ERR, "Unable to locate table %d\n", type);
	}

	return 0;		/* the legacy tool always returns success */
}

static int smbios_info_bios_cmd(struct platform_intf *intf,
                                struct platform_cmd *cmd, int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;

	if (smbios_find_table(intf, SMBIOS_TYPE_BIOS, 0, &table,
	                      SMBIOS_LEGACY_ENTRY_BASE,
	                      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return -1;

	kv = kv_pair_new();
	kv_pair_add(kv, "vendor", table.string[table.data.bios.vendor]);
	kv_pair_add(kv, "version", table.string[table.data.bios.version]);
	kv_pair_add(kv, "release_date",
		    table.string[table.data.bios.release_date]);
	kv_pair_fmt(kv, "size", "%u KB",
		    (table.data.bios.rom_size_64k_blocks + 1) * 64);

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int smbios_info_system_cmd(struct platform_intf *intf,
                                  struct platform_cmd *cmd,
                                  int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;

	if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
	                      SMBIOS_LEGACY_ENTRY_BASE,
	                      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return -1;

	kv = kv_pair_new();
	kv_pair_add(kv, "manufacturer",
		    table.string[table.data.system.manufacturer]);
	kv_pair_add(kv, "name", table.string[table.data.system.name]);
	kv_pair_add(kv, "version", table.string[table.data.system.version]);
	kv_pair_add(kv, "serial_number",
		    table.string[table.data.system.serial_number]);

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int smbios_info_baseboard_cmd(struct platform_intf *intf,
                                     struct platform_cmd *cmd,
                                     int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;

	if (smbios_find_table(intf, SMBIOS_TYPE_BASEBOARD, 0, &table,
	                      SMBIOS_LEGACY_ENTRY_BASE,
	                      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return -1;

	kv = kv_pair_new();
	kv_pair_add(kv, "manufacturer",
		    table.string[table.data.baseboard.manufacturer]);
	kv_pair_add(kv, "product_name", table.string[table.data.baseboard.name]);
	kv_pair_add(kv, "version", table.string[table.data.baseboard.version]);
	kv_pair_add(kv, "serial_number",
		    table.string[table.data.baseboard.serial_number]);

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int smbios_info_log_cmd(struct platform_intf *intf,
			       struct platform_cmd *cmd, int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;
	const char *access_methods[] = {
		"Indexed I/O (8bit)",
		"Indexed I/O (2x8bit)",
		"Indexed I/O (16bit)",
		"Memory Mapped 32bit Address",
		"General Purpose Non-Volatile",
		NULL
	};
	const struct valstr log_status_map[] = {
		{ 0x01, "Valid" },
		{ 0x02, "Full" },
		{ 0x00, NULL }
	};

	if (smbios_find_table(intf, SMBIOS_TYPE_LOG, 0, &table,
	                      SMBIOS_LEGACY_ENTRY_BASE,
	                      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return -1;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "length", "%u bytes", table.data.log.length);
	kv_pair_fmt(kv, "header_start", "0x%04x", table.data.log.header_start);
	kv_pair_fmt(kv, "data_start", "0x%04x", table.data.log.data_start);
	kv_pair_add(kv, "access_method",
		    (table.data.log.method < sizeof(access_methods)) ?
		    access_methods[table.data.log.method] : "Unknown");
	kv_pair_add(kv, "status",
		    val2str(table.data.log.status, log_status_map));
	kv_pair_fmt(kv, "change_token", "0x%08x", table.data.log.change_token);
	kv_pair_fmt(kv, "address", "0x%08x", table.data.log.address.mem);
	kv_pair_fmt(kv, "header_format", "%u", table.data.log.header_format);

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

struct platform_cmd smbios_info_cmds[] = {
	{
		.name	= "bios",
		.desc	= "BIOS Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = smbios_info_bios_cmd }
	},
	{
		.name	= "system",
		.desc	= "System Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = smbios_info_system_cmd }
	},
	{
		.name	= "baseboard",
		.desc	= "Baseboard Information Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = smbios_info_baseboard_cmd }
	},
	{
		.name	= "log",
		.desc	= "Event Log Table",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = smbios_info_log_cmd }
	},
	{ NULL }
};

struct platform_cmd smbios_cmds[] = {
	{
		.name	= "get",
		.desc	= "Legacy String Retrieval",
		.usage	= "<table type> <string number>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = smbios_get_cmd }
	},
	{
		.name	= "info",
		.desc	= "Print SMBIOS Tables",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = smbios_info_cmds }
	},
	{ NULL }
};

struct platform_cmd cmd_smbios = {
	.type	= ARG_TYPE_SUB,
	.name	= "smbios",
	.desc	= "SMBIOS Information",
	.arg	= { .sub = smbios_cmds },
};
