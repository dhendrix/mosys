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
 * output.h: mosys output routines
 */
#ifndef MOSYS_OUTPUT_H__
#define MOSYS_OUTPUT_H__

#include <stdio.h>

/*
 * mosys_printf - standard printf to mosys output file
 *
 * @format:     string format used for performing printf()-like function.
 * @...:        variable argument list for formatting.
 *
 * returns >0 the number of characters printed upon successful return.
 *         <0 if an output error is encountered.
 */
extern int mosys_printf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

/*
 * print_buffer_to_file  -  print raw buffer to FILE* in hex and ascii
 *
 * @fp:         point to FILE
 * @data:       pointer to buffer to print
 * @length:     number of bytes to print
 */
extern void print_buffer_to_file(FILE* fp, void *data, int length);

/*
 * print_buffer  -  print raw buffer to mosys output in hex and ascii
 *
 * @data:       pointer to buffer to print
 * @length:     number of bytes to print
 */
extern void print_buffer(void *data, int length);

#endif /* MOSYS_OUTPUT_H__ */
