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
 *
 * symbols.h: vpd_encode helper functions for symbol handling
 */

#ifndef VPD_ENCODE_SYMBOL_H__
#define VPD_ENCODE_SYMBOL_H__

#include <inttypes.h>

#include "mosys/list.h"

struct vpd_symbol {
	char *name;
	enum {
		BOOLEAN,
		NUMERIC,
		STRING,
	} type;
	char *value;
};

extern int gen_symtab(const char *file);
extern void cleanup_symtab(void);
extern struct vpd_symbol *lookup_symbol(const char *symbol);

/*
 * sym2str - returns value of a symbol as a string
 *
 * @symbol: name of symbol
 *
 * returns symbol's string if successful
 * returns NULL is symbol is not found
 */
extern char *sym2str(const char *symbol);

/*
 * sym2bool - returns value of a symbol as a boolean type
 *
 * @symbol: name of symbol
 *
 * returns 1 if the symbol exists and is set to a non-zero value
 * returns 0 if symbol is set to zero, not found, or not numeric
 */
extern int sym2bool(const char *symbol);

/*
 * update_symbol - add/edit symbol table entry
 *
 * @str:	symbol=value pair to update
 *
 * returns pointer to symbol structure to indicate success
 * returns NULL to indicate failure
 */
extern struct vpd_symbol *update_symbol(const char *str);

/*
 * sym2type0 - extract symbols and append firmware info table to buffer
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * This function is intended to hide tedious symbol extraction steps from
 * higher-level logic.
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
extern int sym2type0(uint16_t handle, uint8_t **buf, size_t len);

/*
 * sym2type1 - extract symbols and append system info table to buffer
 *
 * @handle:	handle for this structure
 * @buf:	buffer to append to
 * @len:	length of buffer
 *
 * This function is intended to hide tedious symbol extraction steps from
 * higher-level logic.
 *
 * returns total size of newly re-sized buffer if successful
 * returns <0 to indicate failure
 */
extern int sym2type1(uint16_t handle, uint8_t **buf, size_t len);

#endif	/* VPD_ENCODE_SYMBOL_H__ */
