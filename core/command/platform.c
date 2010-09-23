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

#include "mosys/platform.h"

#include "mosys/log.h"
#include "mosys/kv_pair.h"

static int print_platforminfo(const char *key, const char *value);

static int platform_vendor_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sysinfo || !intf->cb->sysinfo->vendor)
		return -ENOSYS;

	return print_platforminfo("vendor", intf->cb->sysinfo->vendor(intf));
}

static int platform_name_cmd(struct platform_intf *intf,
                             struct platform_cmd *cmd,
                             int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sysinfo || !intf->cb->sysinfo->name)
		return -ENOSYS;

	return print_platforminfo("name", intf->cb->sysinfo->name(intf));
}

static int platform_version_cmd(struct platform_intf *intf,
			      struct platform_cmd *cmd,
			      int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sysinfo || !intf->cb->sysinfo->version)
		return -ENOSYS;

	return print_platforminfo("version", intf->cb->sysinfo->version(intf));
}

static int platform_family_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sysinfo || !intf->cb->sysinfo->family)
		return -ENOSYS;

	return print_platforminfo("family", intf->cb->sysinfo->family(intf));
}

static int platform_variant_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sysinfo || !intf->cb->sysinfo->variant)
		return -ENOSYS;

	return print_platforminfo("variant", intf->cb->sysinfo->variant(intf));
}

static int print_platforminfo(const char *key, const char *value)
{
	struct kv_pair *kv = kv_pair_new();

	if (!value) {
		lprintf(LOG_ERR, "Unable to determine platform %s\n", key);
		return -1;
	}

	kv_pair_add(kv, key, value);
	kv_pair_print(kv);
	kv_pair_free(kv);
	free((char*)value);
	return 0;
}

struct platform_cmd platform_cmds[] = {
	{
		.name	= "vendor",
		.desc	= "Display Platform Vendor",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_vendor_cmd }
	},
	{
		.name	= "name",
		.desc	= "Display Platform Product Name",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_name_cmd }
	},
	{
		.name	= "version",
		.desc	= "Display Platform Version",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_version_cmd }
	},
	{
		.name	= "family",
		.desc	= "Display Platform Family",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_family_cmd }
	},
	{
		.name	= "variant",
		.desc	= "Display Platform Variant",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_variant_cmd }
	},
	{NULL}
};

struct platform_cmd cmd_platform = {
	.name	= "platform",
	.desc	= "Platform Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = platform_cmds }
};
