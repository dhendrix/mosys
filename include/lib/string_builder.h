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
