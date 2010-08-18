/*
 * Copyright 2010, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistribution of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * Redistribution in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * SUN MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * string.c: string utilities
 */

#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "mosys/alloc.h"
#include "mosys/string.h"

/*
 * val2str_default  -  convert value to string
 *
 * @val:        value to convert
 * @vs:         value-string data
 * @def_str:    default string to return if no matching value found
 *
 * returns pointer to string
 * returns def_str if no matching value found
 */
const char *val2str_default(uint32_t val, const struct valstr *vs,
                            const char *def_str)
{
	int i;

	for (i = 0; vs[i].str; i++) {
		if (vs[i].val == val)
			return vs[i].str;
	}

	return def_str;
}

/*
 * val2str  -  convert value to string
 *
 * @val:        value to convert
 * @vs:         value-string data
 *
 * returns pointer to string
 * returns pointer to "unknown" static string if not found
 */
const char *val2str(uint32_t val, const struct valstr *vs)
{
	return val2str_default(val, vs, "Unknown");
}

/*
 * str2val  -  convert string to value
 *
 * @str:        string to convert
 * @vs:         value-string data
 *
 * returns value for string
 * returns value for last entry in value-string data if not found
 */
uint32_t str2val(const char *str, const struct valstr *vs)
{
	int i;

	for (i = 0; vs[i].str; i++) {
		if (strcasecmp(vs[i].str, str) == 0)
			return vs[i].val;
	}

	return vs[i].val;
}
