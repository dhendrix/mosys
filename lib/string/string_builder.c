/* Copyright 2012, Google Inc.
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
 * string_builder.c: String builder helper functions. See comments in
 * string_builder.h for more details.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "mosys/alloc.h"

#include "lib/string_builder.h"

struct string_builder {
	int buffer_size;
	int used_size;
	char buffer[0];
};

#define DEFAULT_STR_BUFFER_SIZE 128

/* Return the amount of usable space left in the buffer which takes into
 * account a null terminating character being required. */
static int string_builder_space_left(struct string_builder *str_builder)
{
	int space;

	space = str_builder->buffer_size - str_builder->used_size - 1;

	return space;
}


struct string_builder *new_sized_string_builder(int size)
{
	struct string_builder *str_builder;

	if (size <= 0) {
		return NULL;
	}

	str_builder = mosys_malloc(sizeof(*str_builder) + size);

	str_builder->buffer_size = size;
	reset_string_builder(str_builder);

	return str_builder;
}

struct string_builder *new_string_builder(void)
{
	return new_sized_string_builder(DEFAULT_STR_BUFFER_SIZE);
}

struct string_builder *
clone_string_builder(const struct string_builder *str_builder) {
	struct string_builder *new_builder;

	if (str_builder == NULL)
		return NULL;

	new_builder = new_sized_string_builder(str_builder->buffer_size);
	new_builder->used_size = str_builder->used_size;
	memcpy(new_builder->buffer, str_builder->buffer,
	       str_builder->used_size + 1);
	return new_builder;
}

void free_string_builder(struct string_builder *str_builder)
{
	free(str_builder);
}

int reset_string_builder(struct string_builder *str_builder)
{
	if (str_builder == NULL) {
		return -1;
	}

	str_builder->used_size = 0;
	str_builder->buffer[0] = '\0';

	return 0;
}

int string_builder_vsnprintf(struct string_builder *str_builder, size_t size,
                             const char *format, va_list ap)
{
	int space_free;
	int length_written;

	if (str_builder == NULL || format == NULL) {
		return -1;
	}

	space_free = string_builder_space_left(str_builder);

	/* Update how much will be written. space_free includes 1 less than
	 * actual buffer size to accomodate a terminating '\0' byte. Adjust size
	 * to be in terms of space_free bytes. Include the terminating byte in
	 * the size passed to vsnprintf(). */
	size--;
	if (space_free < size) {
		size = space_free;
	}

	length_written = vsnprintf(&str_builder->buffer[str_builder->used_size],
	                           size + 1, format, ap);

	if (length_written > size) {
		/* Handle the case of truncation. */
		str_builder->used_size += size;
		return size;
	} else if (length_written < 0) {
		// COV_NF_START
		/* Shouldn't get an output error from vsnprintf(). Ensure there
		 * is proper null termination just in case. */
		str_builder->buffer[str_builder->used_size] = '\0';
		return -1;
		// COV_NF_END
	} else {
		/* Update the size used. */
		str_builder->used_size += length_written;
	}

	return length_written;
}

int string_builder_snprintf(struct string_builder *str_builder, size_t size,
                            const char *format, ...)
{
	va_list ap;
	int length_written;

	va_start(ap, format);
	length_written = string_builder_vsnprintf(str_builder, size,
	                                          format, ap);
	va_end(ap);

	return length_written;
}

int string_builder_sprintf(struct string_builder *str_builder,
                           const char *format, ...)
{
	va_list ap;
	int space_free;
	int length_written;

	if (str_builder == NULL) {
		return -1;
	}

	space_free = string_builder_space_left(str_builder);

	va_start(ap, format);
	length_written = string_builder_vsnprintf(str_builder,
	                           space_free + 1, format, ap);
	va_end(ap);

	return length_written;
}

int string_builder_strncat(struct string_builder *str_builder, const char *str,
                           size_t size)
{
	int space_left;
	int appended_string_len;

	if (str_builder == NULL || str == NULL) {
		return -1;
	}

	if (size == 0) {
		return 0;
	}

	space_left = string_builder_space_left(str_builder);

	/* Don't copy past the buffer size. */
	if (size > space_left) {
		size = space_left;
	}

	strncpy(&str_builder->buffer[str_builder->used_size], str, size);

	/* Make sure and NULL terminate the string. */
	str_builder->buffer[str_builder->used_size + size] = '\0';

	appended_string_len =
	    strlen(&str_builder->buffer[str_builder->used_size]);

	str_builder->used_size += appended_string_len;

	return appended_string_len;
}

int string_builder_strcat(struct string_builder *str_builder, const char *str)
{
	if (str == NULL) {
		return -1;
	}

	return string_builder_strncat(str_builder, str, strlen(str));
}

int string_builder_add_char(struct string_builder *str_builder, char ch)
{
	if (str_builder == NULL) {
		return -1;
	}

	/* Check if any space is left. */
	if (string_builder_space_left(str_builder) == 0) {
		return -1;
	}

	/* Add character and null terminate. */
	str_builder->buffer[str_builder->used_size++] = ch;
	str_builder->buffer[str_builder->used_size] = '\0';

	return 0;
}

int string_builder_add_hexstring(struct string_builder *str_builder,
                                 char *buffer, int length)
{
	int i;
	int len_written;

	if (str_builder == NULL || buffer == NULL || length < 0) {
		return -1;
	}

	for (i = 0; i < length; i++) {
		len_written = string_builder_sprintf(str_builder, "%02x",
		                                     buffer[i]);
		/* Something really bad happened. */
		if (len_written < 0) {
			return -1; // COV_NF_LINE
		}
		/* Not enough room in the buffer. Reduce back to state before
		 * attempted write. */
		else if (len_written < 2) {
			str_builder->used_size -= len_written;
			str_builder->buffer[str_builder->used_size] = '\0';
			break;
		}
	}

	return i;
}

const char *string_builder_get_string(struct string_builder *str_builder)
{
	if (str_builder == NULL) {
		return NULL;
	}

	return &str_builder->buffer[0];
}
