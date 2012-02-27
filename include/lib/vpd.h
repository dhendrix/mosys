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
