/*
 * Copyright (C) 2011 Google Inc.
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

#include "mosys/alloc.h"
#include "mosys/platform.h"

#include "lib/probe.h"

#if 0
static const char *aebl_tegra2_get_vendor(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *aebl_tegra2_get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);

	return (const char *)ret;
}

#if 0
static const char *aebl_tegra2_get_family(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *aebl_tegra2_get_version(struct platform_intf *intf)
{
	return extract_cpuinfo("Revision");
}

struct sys_cb aebl_tegra2_sys_cb = {
//	.vendor		= &aebl_tegra2_get_vendor,
	.name		= &aebl_tegra2_get_name,
//	.family		= &aebl_tegra2_get_family,
	.version	= &aebl_tegra2_get_version,
};
