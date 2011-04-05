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

#include <ctype.h>
#include <stdio.h>
#include <limits.h>

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/platform.h"

#ifndef LINE_MAX
#define LINE_MAX	512
#endif

static const char *seaboard_tegra2_get_vendor(struct platform_intf *intf)
{
	return mosys_strdup("NVIDIA");
}

static const char *seaboard_tegra2_get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);
	return (const char *)ret;
}

#if 0
static const char *seaboard_tegra2_get_family(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

/* FIXME: this duplicates a lot of code from probe_cpuinfo. */
static const char *seaboard_tegra2_get_version(struct platform_intf *intf)
{
	FILE *cpuinfo;
	char *ret = NULL;
	char path[PATH_MAX];
	char key[] = "Revision";

	sprintf(path, "%s/proc/cpuinfo", mosys_get_root_prefix());
	cpuinfo = fopen(path, "rb");
	if (!cpuinfo)
		return 0;

	while (!feof(cpuinfo)) {
		char line[LINE_MAX], *ptr;
		int i = 0;

		if (fgets(line, sizeof(line), cpuinfo) == NULL)
			break;
		ptr = line;

		if (strncmp(ptr, key, strlen(key)))
			continue;

		ptr += strlen(key);
		while (isspace((unsigned char)*ptr) || (*ptr == ':'))
			ptr++;

		ret = mosys_malloc(strlen(ptr) + 1);
		while (!isspace((unsigned char)*ptr)) {
			ret[i] = *ptr;
			ptr++;
			i++;
		}
		ret[i] = '\0';
	}

	fclose(cpuinfo);
	return (const char *)ret;
}

struct sysinfo_cb seaboard_tegra2_sysinfo_cb = {
	.vendor		= &seaboard_tegra2_get_vendor,
	.name		= &seaboard_tegra2_get_name,
//	.family		= &seaboard_tegra2_get_family,
	.version	= &seaboard_tegra2_get_version,
};
