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
		errno = EINVAL;
		return -1;
	}

	addr = strtoull(argv[0], NULL, 0);
	length = strtoul(argv[1], NULL, 0);

	if (length == 0) {
		platform_cmd_usage(cmd);
		errno = EINVAL;
		return -1;
	}

	mmio_dump(intf, addr, length);

	return 0;
}

static int memory_print_speed(struct platform_intf *intf,
			      struct platform_cmd *cmd, int argc, char **argv)
{
	unsigned long dimm;
	struct kv_pair *kv;
	int rc;

	if (argc != 1) {
		platform_cmd_usage(cmd);
		errno = EINVAL;
		return -1;
	}

	dimm = strtoul(argv[0], NULL, 0);

	if (!intf->cb->memory || !intf->cb->memory->dimm_speed) {
		errno = ENOSYS;
		return -1;
	}

	kv = kv_pair_new();
	intf->cb->memory->dimm_speed(intf, dimm, kv);
	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

/* SMBIOS-based DIMM information */
//extern struct platform_cmd smbios_memory_dimm_cmds[];

/* SPD information */
extern struct platform_cmd memory_spd_cmds[];

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
	{
		.name	= "speed",
		.desc	= "Print Memory Speed",
		.usage	= "print speed <dimmN>",
		.type	= ARG_TYPE_GETTER,
		.arg 	= { .func = memory_print_speed }
	},
	{ NULL },
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
	{
		.name	= "print",
		.desc	= "Print generic information",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_print_cmds }
	},
	{
		.name	= "spd",
		.desc	= "Information from SPD",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_spd_cmds }
	},
	{ NULL }
};

struct platform_cmd cmd_memory = {
	.name	= "memory",
	.desc	= "Memory Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = memory_cmds }
};
