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
 * String helper functions.
 *
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
