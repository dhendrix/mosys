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
 * symbols.h: vpd_encode helper functions for symbol handling
 */

#ifndef VPD_ENCODE_SYMBOL_H__
#define VPD_ENCODE_SYMBOL_H__

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

#endif	/* VPD_ENCODE_SYMBOL_H__ */
