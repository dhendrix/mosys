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

#if defined(__linux__)
#include <limits.h>
#elif defined(__DARWIN__)
#include <sys/syslimits.h>
#endif

#endif /* MOSYS_GLOBALS_H__ */
