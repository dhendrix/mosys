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
 * globals.c: all global variables are declared here.
 */

#include <stdio.h>

#include "mosys/globals.h"

void mosys_globals_init() {
	mosys_set_output_file(stdout);
}

/*
 * The global root-prefix
 */
static const char *mosys_root_prefix = "";

const char *mosys_get_root_prefix(void)
{
	return mosys_root_prefix;
}

void mosys_set_root_prefix(const char *prefix)
{
	if (prefix == NULL) {
		prefix = "";
	}
	mosys_root_prefix = prefix;
}

/*
 * The global output destination
 */
static FILE *mosys_output_file;

FILE *mosys_get_output_file(void)
{
	if (!mosys_output_file)
		return stdout;
	return mosys_output_file;
}

void mosys_set_output_file(FILE *fp)
{
	mosys_output_file = fp;
}

/*
 * The global verbosity level
 */
static int mosys_verbosity;

int mosys_get_verbosity(void)
{
	return mosys_verbosity;
}

void mosys_set_verbosity(int verbosity)
{
	mosys_verbosity = verbosity;
}
