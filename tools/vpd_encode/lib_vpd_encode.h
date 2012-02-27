/*
 * Copyright 2012, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

extern int build_google_vpd_blob(double version, char outfile[]);

#endif	/* VPD_ENCODE_VENDOR_BLOBS_H__ */
