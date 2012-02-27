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
 * output.c: mosys output routines
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "mosys/globals.h"
#include "mosys/output.h"

/*
 * mosys_printf - standard printf to mosys output file
 *
 * @format:       string format used for performing printf()-like function.
 * @...:          variable argument list for formatting.
 *
 * returns >0 the number of characters printed upon successful return.
 *         <0 if an output error is encountered.
 */
int mosys_printf(const char *format, ...)
{
	int ret;
	FILE *fp = mosys_get_output_file();
	va_list arglist;

	va_start(arglist, format);
	ret = vfprintf(fp, format, arglist);
	va_end(arglist);

	return ret;
}

/*
 * print_buffer_to_file  -  print raw buffer to FILE* in hex and ascii
 *
 * @fp:         point to FILE
 * @data:       pointer to buffer to print
 * @length:     number of bytes to print
 */
void print_buffer_to_file(FILE* fp, void *data, int length)
{
	uint8_t astr[20], val;
	int i, ctr;
	int extra = 0;
	uint8_t *buffer = data;

	memset(astr, 0, sizeof(astr));

	/* check for 16-byte unaligned data length */
	if (length % 16)
		extra = 16 - (length % 16);

	for (i = ctr = 0; ctr < length; ctr++) {
		if ((ctr % 16) == 0) {
			if (ctr != 0)
				fprintf(fp, "\n");
			fprintf(fp, "%08x  ", ctr);
		} else if ((ctr % 8) == 0)
			fprintf(fp, " ");

		val = buffer[ctr];
		fprintf(fp, "%02x ", val);

		/* add to ascii string */
		astr[i++] = ((val > 0x1f) && (val < 0x7f)) ? val : 0x2e;

		if (extra && ctr == (length - 1)) {
			/* special handling for unaligned end of data */
			if (extra >= 8)
				fprintf(fp, " ");
			for (; i < 16; i++) {
				fprintf(fp, "   ");
				astr[i] = ' ';
			}
			fprintf(fp, " |%s|", astr);
		} else {
			/* print ascii string at end of hex line */
			if (((ctr + 1) % 16) == 0) {
				fprintf(fp, " |%s|", astr);
				memset(astr, 0, sizeof(astr));
				i = 0;
			}
		}
	}

	fprintf(fp, "\n");
	fflush(fp);
}

/*
 * print_buffer  -  print raw buffer to mosys output in hex and ascii
 *
 * @data:       pointer to buffer to print
 * @length:     number of bytes to print
 */
void print_buffer(void *data, int length)
{
	FILE *fp = mosys_get_output_file();
	print_buffer_to_file(fp, data, length);
}
