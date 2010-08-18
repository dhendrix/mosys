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
 * String helper functions.
 */

#ifndef LIB_STRING_H_
#define LIB_STRING_H_

/* FIXME: Should we #include string_builder.h here? */

/*
 * convert an array of bytes to a hexadecimal string (no prefix)
 *
 * inbuf:    pointer to byte array
 * length:   size of byte array
 *
 * returns a pointer to a dynamically allocated string on success
 * returns NULL on failure
 */
extern char *buf2str(uint8_t *inbuf, size_t length);

/*
 * Build a string, as through sprintf().  The returned string is
 * dynamically allocated and must be free()ed by the caller.
 */
extern char *format_string(const char *fmt, ...)
    __attribute__ ((format(printf, 1, 2)));

/*
 * Scan memory at address=haystack for a block of memory that matches
 * needle.
 */
extern int find_pattern(void *haystack, size_t haystack_length,
                        void *needle, size_t needle_length,
                        size_t align, size_t *offset);

/*
 * Return a pointer to a dynamically allocated string representing the nth
 * field, delimited by delim, in str.
 */
extern char *strfield(const char *str, char delim, int n);

/*
 * Find the min-length string.
 */
#ifndef __minlen
# define __minlen(a, b) ({ int x=strlen(a); int y=strlen(b); (x < y) ? x : y;})
#endif

/*
 * Find the max-length string.
 */
#ifndef __maxlen
# define __maxlen(a, b) ({ int x=strlen(a); int y=strlen(b); (x > y) ? x : y;})
#endif

#endif /* LIB_STRING_H_ */
