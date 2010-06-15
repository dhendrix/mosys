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

#ifndef MOSYS_INTF_LIST_H__
#define MOSYS_INTF_LIST_H__

/* common operations */
struct platform_intf;
struct platform_op;

extern struct platform_op platform_common_op;

/*
 * intf_op_setup  -  prepare interface operations
 *
 * @intf:       platform interface
 */
extern int intf_op_setup(struct platform_intf *intf);

/*
 * intf_op_destroy  -  clean up interface operations
 *
 * @intf:       platform interface
 */
extern void intf_op_destroy(struct platform_intf *intf);

#endif /* MOSYS_INTF_LIST_H__ */
