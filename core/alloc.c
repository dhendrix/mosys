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
 * alloc.c: Memory allocation wrappers.  See comments in alloc.h for details.
 */

#include "mosys/alloc.h"
#include "mosys/log.h"

/* the app-specified allocation failure handler - NULL by default */
static alloc_failure_handler do_alloc_failure;

/* set the allocation failure handler - for use by library users */
void
mosys_set_alloc_failure_handler(alloc_failure_handler func_ptr)
{
	do_alloc_failure = func_ptr;
}

/* the last resort if an allocation fails - this function does not return */
static void *
alloc_fail_abort(size_t size, const char *file, int line, const char *func)
{
	lprintf(LOG_EMERG, "allocation failure in %s (%s:%d): %u bytes\n",
	    func, file, line, (unsigned)size);
	abort();
	return NULL;
}

void *
internal_mosys_malloc(size_t size,
                     const char *file, int line, const char *func)
{
	do {
		void *p = malloc(size);
		if (p != NULL)
			return p;
	} while (do_alloc_failure
	     && (do_alloc_failure(size, file, line, func) == 0));

	return alloc_fail_abort(size, file, line, func);
}

void *
internal_mosys_calloc(size_t nmemb, size_t size,
                     const char *file, int line, const char *func)
{
	do {
		void *p = calloc(nmemb, size);
		if (p != NULL)
			return p;
	} while (do_alloc_failure
	     && (do_alloc_failure(nmemb*size, file, line, func) == 0));

	return alloc_fail_abort(nmemb*size, file, line, func);
}

void *
internal_mosys_zalloc(size_t size,
                     const char *file, int line, const char *func)
{
	return internal_mosys_calloc(1, size, file, line, func);
}

void *
internal_mosys_realloc(void *ptr, size_t size,
                      const char *file, int line, const char *func)
{
	do {
		void *p = realloc(ptr, size);
		if (p != NULL)
			return p;
	} while (do_alloc_failure
	     && (do_alloc_failure(size, file, line, func) == 0));

	return alloc_fail_abort(size, file, line, func);
}

char *
internal_mosys_strdup(const char *str,
                     const char *file, int line, const char *func)
{
	do {
		char *p = strdup(str);
		if (p != NULL)
			return p;
	} while (do_alloc_failure
	     // COV_NF_START
	     && (do_alloc_failure(strlen(str)+1, file, line, func) == 0));

	return alloc_fail_abort(strlen(str)+1, file, line, func); // COV_NF_END
}
