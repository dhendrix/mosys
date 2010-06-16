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
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "mosys/common.h"
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
	{ NULL }
};

struct platform_cmd cmd_nvram = {
	.name	= "nvram",
	.desc	= "NVRAM information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = nvram_cmds }
};
