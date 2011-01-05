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

#ifndef MOSYS_LIB_PROBE_H__
#define MOSYS_LIB_PROBE_H__

struct platform_intf;

/*
 * probe_hwid - attempt to match chromeos hardware id
 *
 * @hwids:	null-terminated list of hardware IDs
 *
 * returns 1 to indicate match
 * returns 0 to indicate no match
 * returns <0 to indicate error
 */
extern int probe_hwid(const char *hwids[]);

/*
 * probe_smbios - probe smbios for system info
 *
 * @ids:	null-terminated list of ids
 *
 * returns 1 to indicate match
 * returns 0 to indicate no match
 * returns <0 to indicate error
 */
extern int probe_smbios(struct platform_intf *intf, const char *ids[]);

#endif /* MOSYS_LIB_PROBE_H__ */
