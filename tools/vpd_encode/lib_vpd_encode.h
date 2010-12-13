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

#ifndef VPD_ENCODE_VENDOR_BLOBS_H__
#define VPD_ENCODE_VENDOR_BLOBS_H__

extern struct vpd_entry *vpd_create_eps(uint8_t major_ver,
                                        uint8_t minor_ver,
                                        uint16_t structure_table_length,
                                        uint32_t structure_table_address,
                                        uint16_t num_structures);
extern int vpd_append_type0(uint16_t handle, uint8_t **buf, size_t len,
                            char *vendor, char *version, uint16_t start,
			    char *date, uint8_t rom_size, uint8_t major_ver,
			    uint8_t minor_ver, uint8_t ec_major_ver,
			    uint8_t ec_minor_ver);
extern int vpd_append_type1(uint16_t handle, uint8_t **buf, size_t len,
                            char *manufacturer, char *name, char *version,
                            char *serial_number, char *uuid, char *sku,
			    char *family);
extern int vpd_append_type127(uint16_t handle, uint8_t **buf, size_t len);

/* FIXME: this should go elsewhere */
void print_buf(enum log_levels threshold, void *buf, size_t len);

#endif	/* VPD_ENCODE_VENDOR_BLOBS_H__ */
