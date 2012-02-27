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
