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
 * globals.h: all global variables are declared here.
 */

#ifndef MOSYS_GLOBALS_H__
#define MOSYS_GLOBALS_H__

/*
 * init all mosys global settings
 */
void mosys_globals_init(void);

/*
 * manage the global filesystem root-prefix (for testing)
 */
extern const char *mosys_get_root_prefix(void);
extern void mosys_set_root_prefix(const char *prefix);

/*
 * manage the global output file
 */
#include <stdio.h>
extern FILE *mosys_get_output_file(void);
extern void mosys_set_output_file(FILE *fp);

/*
 * manage the global verbosity
 */
extern int mosys_get_verbosity(void);
extern void mosys_set_verbosity(int verbosity);

/*
 * OS detection
 */
#if (defined(__MACH__) && defined(__APPLE__))
#define __DARWIN__
#endif		/* OS detection */

#endif /* MOSYS_GLOBALS_H__ */
