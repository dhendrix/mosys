/* Copyright 2012, Google Inc.
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
