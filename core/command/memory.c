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
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/mmio.h"

static int memory_dump_cmd(struct platform_intf *intf,
			   struct platform_cmd *cmd, int argc, char **argv)
{
	uint64_t addr;
	uint32_t length;

	if (argc < 2) {
		platform_cmd_usage(cmd);
		return -1;
	}

	addr = strtoull(argv[0], NULL, 0);
	length = strtoul(argv[1], NULL, 0);

	if (length == 0) {
		platform_cmd_usage(cmd);
		return -1;
	}

	mmio_dump(intf, addr, length);

	return 0;
}

/* SMBIOS-based DIMM information */
//extern struct platform_cmd smbios_memory_dimm_cmds[];

/* SPD information */
//extern struct platform_cmd memory_spd_cmds[];

/* AMB information */
//extern struct platform_cmd memory_amb_cmds[];

/* Error information */
//extern struct platform_cmd memory_error_cmds[];

/* Address conversion */
//extern struct platform_cmd memory_convert_cmds[];

struct platform_cmd memory_print_cmds[] = {
#if 0
	{
		.name	= "all",
		.desc	= "Print all general info",
		.usage	= "print all <dimmN>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = print_all }
	},
	{
		.name	= "id",
		.desc	= "Print module ID info",
		.usage	= "print id <dimmN>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = print_module_id }
	},
#endif
#if 0
	{
		.name	= "pop",
		.desc	= "Population information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = print_memory_population }
	},
#endif
};

struct platform_cmd memory_cmds[] = {
#if 0
	{
		.name	= "smbios",
		.desc	= "SMBIOS-provided information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = smbios_memory_dimm_cmds }
	},
#endif
#if 0
	{
		.name	= "error",
		.desc	= "Error information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_error_cmds }
	},
#endif
#if 0
	{
		.name	= "spd",
		.desc	= "Raw SPD information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_spd_cmds }
	},
#endif
#if 0
	{
		.name	= "amb",
		.desc	= "Raw AMB information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_amb_cmds }
	},
#endif
#if 0
	{
		.name	= "convert",
		.desc	= "Address conversion",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_convert_cmds }
	},
#endif
	{
		.name	= "dump",
		.desc	= "Dump a range of memory",
		.usage	= "<address> <length>\n\n" "example: 0xffdec000 256",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = memory_dump_cmd }
	},
#if 0
	{
		.name	= "print",
		.desc	= "Print generic information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_print_cmds }
	},
#endif
	{ NULL }
};

struct platform_cmd cmd_memory = {
	.name	= "memory",
	.desc	= "Memory Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = memory_cmds }
};
