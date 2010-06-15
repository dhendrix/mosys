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
