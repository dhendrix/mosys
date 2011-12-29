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
 * nm10.c: NM10 helper functions
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "intf/mmio.h"
#include "intf/pci.h"

#include "drivers/intel/ich_generic.h"
#include "drivers/intel/nm10.h"

enum ich_bbs_ich7 nm10_get_bbs(struct platform_intf *intf)
{
	enum ich_bbs_ich7 val;

	if ((val = ich_get_bbs(intf)) < 0)
		return ICH7_BBS_UNKNOWN;

	return val;
}

int nm10_set_bbs(struct platform_intf *intf, enum ich_bbs_ich7 bbs)
{
	if (bbs == ICH7_BBS_UNKNOWN)
		return -1;

	return ich_set_bbs(intf, bbs);
}
