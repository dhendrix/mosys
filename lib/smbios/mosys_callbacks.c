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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/list.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "mosys/string.h"

#include "lib/file.h"
#include "lib/smbios.h"
#include "lib/smbios_tables.h"

#define	SYSFS_SMBIOS_DIR	"/sys/class/dmi/id"

/*
 * smbios_scan_sysfs - scan for a SMBIOS entry by filename and return its value
 *
 * @filename:	filename to scan for
 *
 * Note: User must remember to free allocated memory
 *
 * returns pointer to allocated memory containing SMBIOS info
 * returns NULL if not found
 */
static char *smbios_scan_sysfs(const char *filename)
{
	struct ll_node *list = NULL, *head;
	int fd = 0;
	char *val = NULL;
	char *path;

	list = scanft(&list, SYSFS_SMBIOS_DIR, filename, NULL, 1);
	if (!list_count(list))
		goto smbios_scan_sysfs_exit;

	/* Note: this assumes 1 node in the list (1 SMBIOS table in sysfs) */
	head = list_head(list);
	path = (char *)head->data;

	if ((fd = open(path, O_RDONLY)) < 0) {
		lprintf(LOG_DEBUG, "Error opening %s: %s\n",
		        path, strerror(errno));
		goto smbios_scan_sysfs_exit;
	}

	val = mosys_malloc(SMBIOS_MAX_STRING_LENGTH);
	memset(val, 0, SMBIOS_MAX_STRING_LENGTH);
	if (read(fd, val, SMBIOS_MAX_STRING_LENGTH) < 0) {
		lprintf(LOG_DEBUG, "Error reading %s: %s\n",
		        path, strerror(errno));
		free(val);
		val = NULL;
		goto smbios_scan_sysfs_exit;
	}

smbios_scan_sysfs_exit:
	if (fd)
		close(fd);

	return val;
}

/*
 * smbios_sysinfo_get_vendor  -  return platform vendor
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform vendor string
 * returns NULL if not found
 */
static char *smbios_sysinfo_get_vendor(struct platform_intf *intf)
{
	char *str = NULL;

	/* try normal SMBIOS approach first */
	str = smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 0,
	                         SMBIOS_LEGACY_ENTRY_BASE,
	                         SMBIOS_LEGACY_ENTRY_LEN);

	if (!str) {
		lprintf(LOG_DEBUG, "%s: normal method failed, trying sysfs\n");
		str = smbios_scan_sysfs("sys_vendor");
	}

	return str;
}

/*
 * smbios_sysinfo_get_name  -  return platform product name
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform name string
 * returns NULL if not found
 */
static char *smbios_sysinfo_get_name(struct platform_intf *intf)
{
	char *str = NULL;

	str = smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 1,
	                         SMBIOS_LEGACY_ENTRY_BASE,
	                         SMBIOS_LEGACY_ENTRY_LEN);

	if (!str) {
		lprintf(LOG_DEBUG, "%s: attempting to use sysfs\n");
		str = smbios_scan_sysfs("product_name");
	}

	return str;
}

/*
 * smbios_sysinfo_get_version  -  return platform version
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform version string
 * returns NULL if not found
 */
static char *smbios_sysinfo_get_version(struct platform_intf *intf)
{
	char *str = NULL;

	str = smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 2,
	                         SMBIOS_LEGACY_ENTRY_BASE,
	                         SMBIOS_LEGACY_ENTRY_LEN);

	if (!str) {
		lprintf(LOG_INFO, "%s: normal approach failed, trying sysfs\n");
		str = smbios_scan_sysfs("product_version");
	}

	return str;
}

/*
 * smbios_sysinfo_get_family  -  return platform family
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform version string
 * returns NULL if not found
 */
static char *smbios_sysinfo_get_family(struct platform_intf *intf)
{
	return smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 5,
	                          SMBIOS_LEGACY_ENTRY_BASE,
	                          SMBIOS_LEGACY_ENTRY_LEN);
}

/*
 * smbios_sysinfo_get_family  -  return platform family
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform version string
 * returns NULL if not found
 */
static char *smbios_sysinfo_get_sku(struct platform_intf *intf)
{
	return smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 4,
	                          SMBIOS_LEGACY_ENTRY_BASE,
	                          SMBIOS_LEGACY_ENTRY_LEN);
}

struct smbios_cb smbios_sysinfo_cb = {
	.system_vendor		= smbios_sysinfo_get_vendor,
	.system_name		= smbios_sysinfo_get_name,
	.system_version		= smbios_sysinfo_get_version,
	.system_family		= smbios_sysinfo_get_family,
	.system_sku		= smbios_sysinfo_get_sku,
};
