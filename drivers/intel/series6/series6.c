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
 *
 * series6.c: Intel 6-series chipset helper functions. For now, this is
 * trivial wrappers around similar NM10 helper functions.
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "drivers/intel/nm10.h"
#include "drivers/intel/series6.h"

enum ich_bbs series6_get_bbs(struct platform_intf *intf)
{
	return nm10_get_bbs(intf);
}

int series6_set_bbs(struct platform_intf *intf, enum ich_bbs bbs)
{
	return nm10_set_bbs(intf, bbs);
}
