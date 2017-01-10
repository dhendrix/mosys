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

#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "mosys/alloc.h"
#include "mosys/log.h"

#include "lib/string.h"

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

/* Right-shift nibbles in an array by 1 (4 bits) */
void rshift_nibbles(uint8_t array[], size_t len)
{
	int i;

	for (i = len; i >= 0; i--) {
		if (i == 0)
			array[i] = array[i] >> 4;
		else
			array[i] = (array[i - 1] << 4) | (array[i] >> 4);
	}
}

/* Left-shift nibbles in an array by 1 (4 bits) */
void lshift_nibbles(uint8_t array[], size_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i == len - 1)
			array[i] = array[i] << 4;
		else
			array[i] = (array[i + 1] >> 4) | (array[i] << 4);
	}
}

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
int nstr2buf(uint8_t **buf, const char *str, int base, const char *delim)
{
	unsigned char c[2] = { '\0', '\0' };
	unsigned long int digit;
	int str_idx, tmp_idx, len, err = 0;
	char *endptr;
	uint8_t *tmp;
	enum {
		low,
		high,
	} nibble = high;

	if (!str || ((base < 2) || (base > 16)))
		return -1;

	len = strlen(str);
	tmp = mosys_malloc(len);	/* more than we need */
	memset(tmp, 0, len);

	tmp_idx = 0;
	for (str_idx = 0; str_idx < len; str_idx++) {
		c[0] = str[str_idx];

		digit = strtol(c, &endptr, base);
		if (*endptr != '\0') {
			/* perhaps we hit a delimiter? */
			if (!strpbrk(&c[0], delim)) {
				lprintf(LOG_ERR, "invalid character: "
				                 "\'%c\'\n", c[0]);
				err = 1;
				goto nstr2buf_exit;
			} else {
				continue;
			}
		}

		/* FIXME: superfluous debug print */
		//lprintf(LOG_DEBUG, "%s: str_idx: %d, char: \'%c\'\n",
		//                   __func__, str_idx, c[0]);
		if (nibble == high) {
			tmp[tmp_idx] |= digit << 4;
			nibble = low;
		} else {
			tmp[tmp_idx] |= digit;
			nibble = high;
			tmp_idx++;
		}
	}

	/* if we ended the loop with nibble == low, then we must ensure
	 * that the final byte is counted and align the array */
	if (nibble == low) {
		tmp_idx++;
		rshift_nibbles(tmp, tmp_idx);
	}

	/* tmp_idx represents the actual length of the buffer */
	*buf = mosys_malloc(tmp_idx);
	memset(*buf, 0, tmp_idx);
	memcpy(*buf, tmp, tmp_idx);

nstr2buf_exit:
	free(tmp);
	if (err)
		return -1;
	return tmp_idx;
}

/* returns an allocated buffer containing network device ID string if
   successful, NULL otherwise */
char *buf2nicid(uint8_t *inbuf, enum nic_id_type type)
{
	char *outbuf = NULL;

	if (inbuf == NULL)
		return NULL;

	switch(type){
	case NIC_ID_IEEE802:
		/*
		 * Each raw byte will contain two hex digits. We also need 5
		 * bytes for semi-colons (':') and one for the terminator.
		 */
		outbuf = mosys_malloc((2 * NIC_ID_IEEE802_LENGTH) + 5 + 1);
		sprintf(outbuf, "%02x:%02x:%02x:%02x:%02x:%02x",
	                inbuf[0], inbuf[1], inbuf[2], inbuf[3], inbuf[4],
			inbuf[5]);

		break;
	case NIC_ID_IMEI:{
		int len = (2 * NIC_ID_IMEI_LENGTH) + 3 + 1;
		unsigned char tmp[NIC_ID_IMEI_LENGTH];

		/*
		 * Each raw byte will contain two hex digits. We also need 3
		 * bytes for dashes ('-') and one for the terminator.
		 */
		outbuf = mosys_malloc(len);

		/* The IMEI is a strange format... We need to shift all nibbles
		 * left by one and truncate one nibble from the check digit */
		memcpy(&tmp[0], inbuf, NIC_ID_IMEI_LENGTH);
		lshift_nibbles(tmp, NIC_ID_IMEI_LENGTH);
		sprintf(outbuf, "%02x-%02x%02x%02x-%02x%02x%02x-%x",
	                tmp[0], tmp[1], tmp[2], tmp[3], tmp[4],
			tmp[5], tmp[6], tmp[7] >> 4);
		break;
	}
	default:
		lprintf(LOG_DEBUG, "%s: unsupported NIC ID type: %d\n",
		                   __func__, type);
		return NULL;
	}

	return outbuf;
}

const char *strlfind(const char *str, const char *arr[], int cs)
{
	int i;
	const char *ret = NULL;

	if (cs < 0 || cs > 1) {
		lprintf(LOG_SPEW, "Invalid CS\n");
		return NULL;
	}

	if (arr == NULL) {
		lprintf(LOG_SPEW, "Empty static id list.\n");
		return NULL;
	}

	for (i = 0; arr[i] != NULL; i++) {
		lprintf(LOG_SPEW, "%s: \"%s\" ?= \"%s\": ",
		                  __func__, str, arr[i]);
		if (cs) {
			if (!strncmp(str, arr[i], strlen(arr[i]))) {
				ret = arr[i];
				lprintf(LOG_SPEW, "Yes.\n");
				break;
			}
		} else {
			if (!strncasecmp(str, arr[i], strlen(arr[i]))) {
				ret = arr[i];
				lprintf(LOG_SPEW, "Yes.\n");
				break;
			}
		}

		lprintf(LOG_SPEW, "No.\n");
	}

	return ret;
}

char *strlower(char *str)
{
	char *p;

	for (p = str; *p; p++)
		*p = tolower(*p);

	return str;
}
