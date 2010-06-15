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

#include <stdlib.h>
#include <string.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "mosys/string.h"

#include "lib/smbios.h"
#include "lib/smbios_tables.h"

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
	return smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 0,
	                          SMBIOS_LEGACY_ENTRY_BASE,
	                          SMBIOS_LEGACY_ENTRY_LEN);
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
	return smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 1,
	                          SMBIOS_LEGACY_ENTRY_BASE,
	                          SMBIOS_LEGACY_ENTRY_LEN);
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
	return smbios_find_string(intf, SMBIOS_TYPE_SYSTEM, 2,
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
