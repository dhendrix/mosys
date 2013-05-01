/* Copyright 2012, Google Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <valstr.h>

#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"

#include "lib/smbios.h"
#include "lib/smbios_tables.h"

static int smbios_get_cmd(struct platform_intf *intf,
			  struct platform_cmd *cmd, int argc, char **argv)
{
	uint8_t type, num;
	char *str;
	int rc = 0;

	/* this is legacy tool format */
	if (argc < 2) {
		platform_cmd_usage(cmd);
		errno = EINVAL;
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
		rc = kv_pair_print(kv);
		kv_pair_free(kv);
		free(str);
	} else {
		lprintf(LOG_ERR, "Unable to locate table %d\n", type);
	}

	return rc;		/* the legacy tool always returns success */
}

static int smbios_info_bios_cmd(struct platform_intf *intf,
                                struct platform_cmd *cmd, int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;
	int rc;

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

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int smbios_info_system_cmd(struct platform_intf *intf,
                                  struct platform_cmd *cmd,
                                  int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;
	int rc;

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

	if (mosys_get_verbosity() > CONFIG_LOGLEVEL) {
		kv_pair_add(kv, "sku_number",
		            table.string[table.data.system.sku_number]);
		kv_pair_add(kv, "family",
		            table.string[table.data.system.family]);
	}

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int smbios_info_baseboard_cmd(struct platform_intf *intf,
                                     struct platform_cmd *cmd,
                                     int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;
	int rc;

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

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int smbios_info_log_cmd(struct platform_intf *intf,
			       struct platform_cmd *cmd, int argc, char **argv)
{
	struct smbios_table table;
	struct kv_pair *kv;
	int rc;
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
	kv_pair_fmt(kv, "valid", "%s",
		    table.data.log.status & 1 ? "yes" : "no");
	kv_pair_fmt(kv, "full", "%s", table.data.log.status & 2 ? "yes" : "no");
	kv_pair_fmt(kv, "change_token", "0x%08x", table.data.log.change_token);
	kv_pair_fmt(kv, "address", "0x%08x", table.data.log.address.mem);
	kv_pair_fmt(kv, "header_format", "%u", table.data.log.header_format);
	kv_pair_fmt(kv, "descriptor_num", "%d", table.data.log.descriptor_num);
	kv_pair_fmt(kv, "descriptor_len", "%d", table.data.log.descriptor_len);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
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
