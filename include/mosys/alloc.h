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
 * alloc.h: Memory allocation wrappers.
 *
 * The intention of this module is that none of the mosys core code needs
 * to check for a failure to allocate memory.  All these functions either
 * succeed or abort.
 *
 * To make the behavior friendlier to users of the mosys library, there is
 * an "allocation failure handler" function pointer, which can be
 * installed by the library user.  If any allocation fails, the allocation
 * failure handler will be called.  The allocation failure handler can
 * choose to do any of 3 things.
 *
 * First, it can release some memory and then return 0, in which case the
 * allocation will be retried.  If the allocation fails again, the
 * allocation failure handler will be called again.  As long as the
 * allocation failure handler returns 0, this will loop.
 *
 * Second, it can abort the application itself, in whatever manner is
 * appropriate.  This gives the library user control over the final
 * actions of the app, such as logging errors.
 *
 * Third, it can do whatever it needs to do in preparation for death, such
 * as logging an error, and then return non-zero, in which case the
 * default action will be taken, which will terminate the process.
 */

#ifndef MOSYS_ALLOC_H__
#define MOSYS_ALLOC_H__

#include <string.h>
#include <stdlib.h>

/* memory allocation routines */
extern void *
internal_mosys_malloc(size_t size,
                     const char *file, int line, const char *func);
#define mosys_malloc(size) \
	internal_mosys_malloc((size), __FILE__, __LINE__, __func__)
extern void *
internal_mosys_calloc(size_t nmemb, size_t size,
                     const char *file, int line, const char *func);
#define mosys_calloc(nmemb, size) \
	internal_mosys_calloc((nmemb), (size), __FILE__, __LINE__, __func__)
extern void *
internal_mosys_zalloc(size_t size,
                     const char *file, int line, const char *func);
#define mosys_zalloc(size) \
	internal_mosys_zalloc((size), __FILE__, __LINE__, __func__)
extern void *
internal_mosys_realloc(void *ptr, size_t size,
                      const char *file, int line, const char *func);
#define mosys_realloc(ptr, size) \
	internal_mosys_realloc((ptr), (size), __FILE__, __LINE__, __func__)
extern char *
internal_mosys_strdup(const char *str,
                     const char *file, int line, const char *func);
#define mosys_strdup(str) \
	internal_mosys_strdup((str), __FILE__, __LINE__, __func__)

/* The allocation failure handler funtion signature */
typedef int (*alloc_failure_handler)(size_t size, const char *file, int line,
                                     const char *func);

/* Set the allocation failure handler. */
extern void
mosys_set_alloc_failure_handler(alloc_failure_handler func_ptr);

#endif /* MOSYS_ALLOC_H__ */
