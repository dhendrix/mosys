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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "mosys/log.h"
#include "mosys/platform.h"

static int nvram_clear_cmd(struct platform_intf *intf,
                           struct platform_cmd *cmd, int argc, char **argv)
{
	if (!intf->cb->nvram || !intf->cb->nvram->clear) {
		return -ENOSYS;
	}

	return intf->cb->nvram->clear(intf);
}

static int nvram_list_cmd(struct platform_intf *intf,
                          struct platform_cmd *cmd, int argc, char **argv)
{
	if (!intf->cb->nvram || !intf->cb->nvram->list) {
		return -ENOSYS;
	}

	return intf->cb->nvram->list(intf);
}

static int nvram_dump_cmd(struct platform_intf *intf,
                          struct platform_cmd *cmd, int argc, char **argv)
{
	if (!intf->cb->nvram || !intf->cb->nvram->dump) {
		return -ENOSYS;
	}

	return intf->cb->nvram->dump(intf);
}

static int nvram_vboot_read(struct platform_intf *intf,
                            struct platform_cmd *cmd, int argc, char **argv)
{
	if (!intf->cb->nvram || !intf->cb->nvram->vboot_read)
		return -ENOSYS;

	return intf->cb->nvram->vboot_read(intf);
}

static int nvram_vboot_write(struct platform_intf *intf,
                             struct platform_cmd *cmd, int argc, char **argv)
{
	if (!intf->cb->nvram || !intf->cb->nvram->vboot_write)
		return -ENOSYS;

	if (argc != 1) {
		platform_cmd_usage(cmd);
		return -1;
	}

	return intf->cb->nvram->vboot_write(intf, argv[0]);
}

struct platform_cmd nvram_vboot_cmds[] = {
	{
		.name	= "read",
		.desc	= "Read VbNvContext from NVRAM",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = nvram_vboot_read }
	},
	{
		.name	= "write",
		.desc	= "Write VbNvContext from NVRAM",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = nvram_vboot_write }
	},
	{ NULL }
};

struct platform_cmd nvram_cmds[] = {
	{
		.name	= "clear",
		.desc	= "Clear Configuration",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = nvram_clear_cmd }
	},
	{
		.name	= "dump",
		.desc	= "Dump NVRAM",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = nvram_dump_cmd }
	},
	{
		.name	= "list",
		.desc	= "List NVRAM Variables",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = nvram_list_cmd }
	},
	{
		.name	= "vboot",
		.desc	= "Access VbNvContext on NVRAM",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = nvram_vboot_cmds }
	},
	{ NULL }
};

struct platform_cmd cmd_nvram = {
	.name	= "nvram",
	.desc	= "NVRAM information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = nvram_cmds }
};
