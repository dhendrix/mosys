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

#ifndef MOSYS_LIB_STRING_H_
#define MOSYS_LIB_STRING_H_

#include <inttypes.h>

/* FIXME: Should we #include string_builder.h and valstr.h here? */

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
 * rshift_nibbles - Right-shift nibbles in an array by 1 (4 bits)
 *
 * @array:	array to shift
 * @len:	length of array
 */
extern void rshift_nibbles(uint8_t array[], size_t len);

/*
 * lshift_nibbles - Left-shift nibbles in an array by 1 (4 bits)
 *
 * @array:	array to shift
 * @len:	length of array
 */
extern void lshift_nibbles(uint8_t array[], size_t len);

/*
 * Return a pointer to a dynamically allocated string representing the nth
 * field, delimited by delim, in str.
 */
extern char *strfield(const char *str, char delim, int n);

/**
 * nstr2buf - convert a string of numerical characters into binary form
 *
 * @buf: 	buffer to store result
 * @str:	null-terminated string to convert
 * @base:	number base to convert to, range is 2 to 16
 * @delim:	string containing all delimiting characters
 *
 * This function will store the content of the provided string into a buffer.
 * Delimiting characters will be left out of the result. This is essentially
 * a glorified wrapper around strtoul().
 *
 * returns length of buffer if successful
 * returns <0 to indicate error
 */
extern int nstr2buf(uint8_t **buf, const char *str,
                    int base, const char *delim);

/* Network device ID types */
enum nic_id_type {
	NIC_ID_UNSPECIFIED = 0,
	NIC_ID_IEEE802,
	NIC_ID_IMEI,
	NIC_ID_IMEISV,
	NIC_ID_MEID,
};

/* Network device ID lengths */
enum {
	NIC_ID_IEEE802_LENGTH = 6,
	NIC_ID_IMEI_LENGTH = 8,
	NIC_ID_IMEISV_LENGTH = 8,
	NIC_ID_MEID_LENGTH = 7,
};

 /**
 * buf2nicid - Convert raw data to a formatted, null-terminated NIC ID string
 *
 * @data:	raw data
 * @type:	type of network ID to produce from input buffer
 *
 * For IEEE802.x (MAC-48), output will be in the form "01:23:45:67:89:ab"
 * For IMEI, output will be in the form "22-666666-666666-1"
 *
 * returns an allocated network device ID string if successful
 * returns NULL to indicate failure
 */
extern char *buf2nicid(uint8_t *inbuf, enum nic_id_type type);

/*
 * strlfind - linear search for string in set of strings
 *
 * @str:		string to search for
 * @arr:		array to search
 * @case_sensitive:	boolean for case sensitivity
 *
 * returns pointer to array entry if string is found
 * returns NULL otherwise
 */
const char *strlfind(const char *str, const char *arr[], int case_sensitive);

/*
 * strlower - convert a string to lower case
 *
 * @str:		string to convert (in place)
 * returns pointer to string
 */
char *strlower(char *str);

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

#endif /* MOSYS_LIB_STRING_H_ */
