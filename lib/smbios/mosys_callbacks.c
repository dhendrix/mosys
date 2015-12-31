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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/kv_pair.h"
#include "mosys/list.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/file.h"
#include "lib/smbios.h"
#include "lib/smbios_tables.h"
#include "lib/string.h"

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

	list = scanft(&list, SYSFS_SMBIOS_DIR, filename, NULL, -1, 1);
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
 * smbios_bios_get_vendor  -  return bios vendor
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated bios vendor string
 * returns NULL if not found
 */
char *smbios_bios_get_vendor(struct platform_intf *intf)
{
	char *str = NULL;
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_BIOS, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_DEBUG, "%s: normal method failed, "
		                   "trying sysfs\n", __func__);
		str = smbios_scan_sysfs("bios_vendor");
	} else {
		str = mosys_strdup(table.string[table.data.bios.vendor]);
	}

	return str;
}

/*
 * smbios_sysinfo_get_vendor  -  return platform vendor
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform vendor string
 * returns NULL if not found
 */
char *smbios_sysinfo_get_vendor(struct platform_intf *intf)
{
	char *str = NULL;
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_DEBUG, "%s: normal method failed, "
		                   "trying sysfs\n", __func__);
		str = smbios_scan_sysfs("sys_vendor");
	} else {
		str = mosys_strdup(table.string
				   [table.data.system.manufacturer]);
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
char *smbios_sysinfo_get_name(struct platform_intf *intf)
{
	char *str = NULL;
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_DEBUG, "%s: attempting to use sysfs\n", __func__);
		str = smbios_scan_sysfs("product_name");
	} else {
		str = mosys_strdup(table.string[table.data.system.name]);
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
char *smbios_sysinfo_get_version(struct platform_intf *intf)
{
	char *str = NULL;
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_INFO, "%s: normal approach failed, trying sysfs\n",
		                  __func__);
		str = smbios_scan_sysfs("product_version");
	} else {
		str = mosys_strdup(table.string[table.data.system.version]);
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
char *smbios_sysinfo_get_family(struct platform_intf *intf)
{
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return NULL;

	return mosys_strdup(table.string[table.data.system.family]);
}

/*
 * smbios_sysinfo_get_family  -  return platform family
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform version string
 * returns NULL if not found
 */
char *smbios_sysinfo_get_sku(struct platform_intf *intf)
{
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return NULL;

	return mosys_strdup(table.string[table.data.system.sku_number]);
}

struct smbios_cb smbios_sysinfo_cb = {
	.bios_vendor		= smbios_bios_get_vendor,
	.system_vendor		= smbios_sysinfo_get_vendor,
	.system_name		= smbios_sysinfo_get_name,
	.system_version		= smbios_sysinfo_get_version,
	.system_family		= smbios_sysinfo_get_family,
	.system_sku		= smbios_sysinfo_get_sku,
};

/*
 * Memory callbacks
 */

/*
 * smbios_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
int smbios_dimm_count(struct platform_intf *intf)
{
	int status = 0, dimm_cnt = 0;
	struct smbios_table table;

	while (status == 0) {
		status = smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm_cnt,
					   &table,
					   SMBIOS_LEGACY_ENTRY_BASE,
					   SMBIOS_LEGACY_ENTRY_LEN);
		if (status == 0)
			dimm_cnt++;
	}

	return dimm_cnt;
}

int smbios_dimm_speed(struct platform_intf *intf,
		     int dimm, struct kv_pair *kv)
{
	struct smbios_table table;
	if (smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		return -1;
	}

	kv_pair_fmt(kv, "speed", "%d MHz", table.data.mem_device.speed);

	return 0;
}
