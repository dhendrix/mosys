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

#include "mosys/alloc.h"
#include "mosys/platform.h"

#include "lib/smbios.h"

#include "cr48.h"

static const char *google_cr48_get_vendor(struct platform_intf *intf)
{
	if (intf->cb && intf->cb->smbios)
		return intf->cb->smbios->system_vendor(intf);
	else
		return NULL;
}

static const char *google_cr48_get_name(struct platform_intf *intf)
{
	return (const char *)mosys_strdup(intf->name);
}

static const char *google_cr48_get_family(struct platform_intf *intf)
{
	if (intf->cb && intf->cb->smbios)
		return intf->cb->smbios->system_family(intf);
	else
		return NULL;
}

static const char *google_cr48_get_variant(struct platform_intf *intf)
{
	return google_cr48_ec_mbid(intf);
}

struct sys_cb google_cr48_sys_cb = {
	.vendor		= &google_cr48_get_vendor,
	.name		= &google_cr48_get_name,
	.family		= &google_cr48_get_family,
	.variant	= &google_cr48_get_variant,
};
