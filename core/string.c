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
 * buf2str  -  represent an array of bytes as a hex string (no prefix)
 *
 * @inbuf:      input array[]
 * @length:     buffer length
 *
 * returns allocated pointer to outbuf
 * returns NULL on failure
 */
char *buf2str(uint8_t *inbuf, size_t length)
{
	size_t i;
	char *outbuf;
	char *outp;

	if (inbuf == NULL)
		return NULL;

	outbuf = mosys_malloc(2*length + 1);
	outp = outbuf;

	for (i = 0; i < length; i++) {
		/* sprintf includes the NUL terminator */
		sprintf(outp, "%02x", inbuf[i]);
		outp += 2;
	}

	outbuf[2*length] = '\0';
	return outbuf;
}

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

char *format_string(const char *fmt, ...)
{
	int n;
	int size = 128;
	char *p;
	va_list ap;

	p = mosys_malloc(size);
	while (1) {
		/* try to print in the allocated space */
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);

		/* else try again with more space */
		if (n < 0) {
			/* older libc says overflow = error, double the size */
			size *= 2;  // COV_NF_LINE
		} else if (n >= size) {
			/* newer libs returns precisely what is needed */
			size = n+1;
		} else {
			/* success */
			return p;
		}
		p = mosys_realloc(p, size);
	}
	return NULL;
}

/*
 * find_pattern - find pattern in a buffer
 *
 * @haystack:		buffer to search in
 * @haystack_length:	number of bytes in haystack
 * @needle:		pattern to search for
 * @needle_length:	number of bytes in needle
 * @align:		alignment required
 * @offset:		location of needle, if found
 * 
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int find_pattern(void *haystack, size_t haystack_length,
                 void *needle, size_t needle_length,
                 size_t align, size_t *offset)
{
	size_t i;

	if (!haystack || !needle || !offset) {
		return -1;
	}

	for (i = 0; i < haystack_length; i += align) {
		if (i + needle_length >= haystack_length) {
			break;
		}
		if (!memcmp((uint8_t *)haystack + i, needle, needle_length)) {
			*offset = i;
			return 0;
		}
	}

	return -1;
}

/*
 * strfield - Find the nth field (starting at 0) in a string, and return a copy.
 *
 * @str:   input string to parse by delimiter.
 * @delim: delimiter to use during parsing of input string.
 * @n:     0-based index of substring desired.
 *
 * returns NULL on error, otherwise the substring requested. If the delimiter is
 * not found, NULL is returned.
 */
char *strfield(const char *str, char delim, int n)
{
	char *mystr;
	char *left;
	char *right;
	char *ret = NULL;

	/* sanity checks */
	if (str == NULL || n < 0 || delim == '\0')
		return NULL;

	/* make a local copy of str */
	//mystr = mosys_strdup(str);
	mystr = strdup(str);

	/* find the nth field */
	left = mystr;
	do {
		right = strchr(left, delim);
		if (right) {
			/* there is more string to scan */
			*right = '\0';
			right++;
		} else if (left == mystr) {
			/* Delmiter not found in original input string. */
			break;
		}
		if (n == 0) {
			/* we reached the field we wanted */
			//ret = mosys_strdup(left);
			ret = strdup(left);
			break;
		}
		if (right == NULL || *right == '\0') {
			/* we hit the end of the string */
			break;
		}
		left = right;
	} while (n--);

	free(mystr);
	return ret;
}
