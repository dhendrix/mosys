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

#ifndef MOSYS_DRIVERS_INTEL_NM10_H__
#define MOSYS_DRIVERS_INTEL_NM10_H__

#include "drivers/intel/ich_generic.h"

/*
  * nm10_get_bbs - get bios boot straps (bbs) value
  *
  * @intf:	platform interface
  *
  * returns BBS value to indicate success
  * returns <0 to indicate failure
  */
enum ich_bbs nm10_get_bbs(struct platform_intf *intf);

/*
  * nm10_set_bbs - set bios boot straps (bbs) value
  *
  * @intf:	platform interface
  * @bbs:	bbs value
  *
  * returns 0 to indicate success
  * returns <0 to indicate failure
  */
int nm10_set_bbs(struct platform_intf *intf, enum ich_bbs bbs);

#endif /* MOSYS_DRIVERS_INTEL_NM10_H__ */
