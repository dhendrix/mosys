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

#include "lib/string.h"
#include "lib/vpd.h"
#include "lib/vpd_tables.h"

#if 0
/*
 * vpd_sysinfo_get_vendor  -  return platform vendor
 *
 * @intf:       platform interface
 *
 * returns pointer to allocated platform vendor string
 * returns NULL if not found
 */
static char *vpd_sysinfo_vendor(struct platform_intf *intf)
{
	return vpds_find_string(intf, VPD_TYPE_SYSTEM, 0,
	                        SMBIOS_LEGACY_ENTRY_BASE,
	                        SMBIOS_LEGACY_ENTRY_LEN);
}

}

struct smbios_cb smbios_sysinfo_cb = {
	.vendor		= vpd_sysinfo_vendor,
};
#endif
