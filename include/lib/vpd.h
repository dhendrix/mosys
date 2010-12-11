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
 */

#ifndef MOSYS_LIB_VPD_H__
#define MOSYS_LIB_VPD_H__

#include <inttypes.h>

#include "mosys/platform.h"

#include "vpd_tables.h"

/* VPD Table Types */
enum vpd_types {
	VPD_TYPE_FIRMWARE = 0,
	VPD_TYPE_SYSTEM,
	VPD_TYPE_END			= 127,
	VPD_TYPE_BINARY_BLOB_POINTER	= 241,
};

/* VPD platform information callbacks */
extern struct vpd_cb vpd_cb;

/* memory address VPD will be mapped to after detection */
extern unsigned int vpd_rom_base;
extern unsigned int vpd_rom_size;

/* VPD main API. */
extern int vpd_find_table(struct platform_intf *intf,
                          enum vpd_types type,
                          int instance, struct vpd_table *table,
                          unsigned int baseaddr, unsigned int len);
extern char *vpd_find_string(struct platform_intf *intf,
                             enum vpd_types type, int number,
                             unsigned int baseaddr, unsigned int len);
extern int vpd_get_blob(struct platform_intf *intf,
                        struct vpd_table_binary_blob_pointer *bbp,
                        uint8_t **buf);
extern int vpd_print_blob(struct platform_intf *intf,
                          struct kv_pair *kv, struct vpd_table *table);

#endif /* MOSYS_LIB_VPD_H__ */
