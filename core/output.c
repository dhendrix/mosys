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
