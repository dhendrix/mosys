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
#include <errno.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

static int ec_info(struct platform_intf *intf,
                   struct platform_cmd *cmd, int argc, char **argv)
{
	struct kv_pair *kv;

	/* put ec vendor, name, and firmware version in kv=pair format */
	if (!intf->cb->ec ||
	    !intf->cb->ec->vendor ||
	    !intf->cb->ec->name ||
	    !intf->cb->ec->firmware) {
		return -ENOSYS;
	}

	kv = kv_pair_new();

	kv_pair_add(kv, "vendor", intf->cb->ec->vendor(intf));
	kv_pair_add(kv, "name", intf->cb->ec->name(intf));
	kv_pair_add(kv, "firmware", intf->cb->ec->firmware(intf));

	kv_pair_print(kv);
	kv_pair_free(kv);
	return 0;
}

struct platform_cmd ec_cmds[] = {
	{
		.name	= "info",
		.desc	= "Print basic EC information",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = ec_info}
	},
	/* TODO: add a sub-menu for EC commands */
	{ NULL }
};

struct platform_cmd cmd_ec = {
	.name	= "ec",
	.desc	= "EC information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = ec_cmds }
};
