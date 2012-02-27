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
 * String builder helper functions.
 *
 * The string_builder module aids by handling and consolidating the string
 * generating/handling routines that are used frequently.
 */

#ifndef MOSYS_LIB_STRING_BUILDER_H_
#define MOSYS_LIB_STRING_BUILDER_H_

#include <stdarg.h>

#include "mosys/list.h"

struct string_builder;

/*
 * Allocate a new string_builder instance with a buffer of size size.
 *
 * @size:  size of the buffer to be used for building up a string.
 *
 * returns a new string_builder instance.
 */
extern struct string_builder *new_sized_string_builder(int size);

/*
 * Allocate a new string_builder instance using the default internal size (128)
 * for the buffer space..
 *
 * returns a new string_builder instance.
 */
extern struct string_builder *new_string_builder(void);

/*
 * Clone a string_builder instance with the same buffer size and identical data
 * to another string_builder.
 *
 * @str_builder: original string_builder instance.
 *
 * returns a new string_builder instance or NULL on failure.
 */
extern struct string_builder *
clone_string_builder(const struct string_builder *str_builder);

/*
 * Free a previously allocated string_builder instance.
 *
 * @str_builder:  string_builder instance to free.
 */
extern void free_string_builder(struct string_builder *str_builder);

/*
 * Reset the internal state of a string_builder instance (used buffer = 0).
 *
 * @str_builder:  string_builder instance to reset internal state.
 *
 * returns 0 on success, < 0 on failure.
 */
extern int reset_string_builder(struct string_builder *str_builder);

/*
 * Fill in a string_builder instance using the semantics of vsnprintf().
 *
 * @str_builder:  string_builder instance add formatted string to.
 * @size:         size of bytes to add including the terminating '\0'.
 * @format:       string format used for performing sprintf()-like function.
 * @...:          variable argument list for formatting.
 *
 * returns -1 on error else the amount of characters written to the builder.
 */
extern int string_builder_vsnprintf(struct string_builder *str_builder,
                                    size_t size, const char *format,
                                    va_list ap);
/*
 * Fill in a string_builder instance using the semantics of snprintf().
 *
 * @str_builder:  string_builder instance add formatted string to.
 * @format:       string format used for performing sprintf()-like function.
 * @...:          variable argument list for formatting.
 *
 * returns -1 on error else the amount of characters written to the builder.
 */
extern int string_builder_snprintf(struct string_builder *str_builder,
                                   size_t size, const char *format, ...)
                                   __attribute__((format(printf, 3, 4)));
/*
 * Fill in a string_builder instance using the semantics of sprintf().
 *
 * @str_builder:  string_builder instance add formatted string to.
 * @format:       string format used for performing sprintf()-like function.
 * @...:          variable argument list for formatting.
 *
 * returns -1 on error else the amount of characters written to the builder.
 */
extern int string_builder_sprintf(struct string_builder *str_builder,
                                  const char *format, ...)
                                   __attribute__((format(printf, 2, 3)));

/*
 * Append a string at the end of a string_builder instance.
 *
 * @str_builder:  string_builder instance to append to.
 * @str:          string to append to the builder
 * @size:         at most bytes to append (not including tailing '\0'
 *
 * returns number of bytes appended on success, < 0 on failure.
 */
extern int string_builder_strncat(struct string_builder *str_builder,
                                 const char *str, size_t size);

/*
 * Append a string at the end of a string_builder instance.
 *
 * @str_builder:  string_builder instance to append to.
 * @str:          string to append to the builder
 *
 * returns number of bytes appended on success, < 0 on failure.
 */
extern int string_builder_strcat(struct string_builder *str_builder,
                                 const char *str);

/*
 * Append a character at the end of a string_builder instance.
 *
 * @str_builder:  string_builder instance to append to.
 * @ch:           character to append to the string_builder instance.
 *
 * returns 0 on success, < 0 on failure.
 */
extern int string_builder_add_char(struct string_builder *str_builder,
                                   char ch);

/*
 * Append a hexadecimal representation of the input buffer at the end of a
 * string_builder instance.
 *
 * @str_builder:  string_builder instance to append to.
 * @ch:           character to append to the string_builder instance.
 *
 * returns the number of bytes converted and added on success, < 0 on failure.
 */
extern int string_builder_add_hexstring(struct string_builder *str_builder,
                                        char *buffer, int length);

/*
 * Get the built string.
 *
 * @str_builder:  string_builder instance to obtain string from.
 *
 * returns the string built by str_builder, or NULL str_builder is invalid.
 */
extern const char *string_builder_get_string(
                       struct string_builder *str_builder);

#endif /* MOSYS_LIB_STRING_BUILDER_H_ */
