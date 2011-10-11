/*
 * Copyright 2011, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
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
 */

#include <inttypes.h>
#include <sys/types.h>

#include "mosys/alloc.h"
#include "mosys/log.h"

#include "lib/dynamic_array.h"

struct dynamic_array {
	size_t size;
	size_t element_size;
	uint8_t *data;
};

struct dynamic_array *new_dynamic_array(size_t element_size)
{
	struct dynamic_array *v = mosys_malloc(sizeof(struct dynamic_array));
	v->data = NULL;
	v->size = 0;
	v->element_size = element_size;
	return v;
}

void free_dynamic_array(struct dynamic_array *v)
{
	if (v)
		free(v->data);
	free(v);
}

int dynamic_array_push_back(struct dynamic_array *v, void *data)
{
	if (!v || !data) {
		lprintf(LOG_ERR, "%s: Invalid argument, v: %p, data: %p",
		                 __func__, v, data);
		return -1;
	}
	v->data = mosys_realloc(v->data, (v->size + 1) * v->element_size);
	memcpy(v->data + v->size * v->element_size, data, v->element_size);
	v->size++;
	return 0;
}

void *dynamic_array_get(const struct dynamic_array *v, size_t i)
{
	if (!v) {
		lprintf(LOG_ERR, "%s: NULL input detected\n", __func__);
		return NULL;
	}
	if (i > v->size) {
		lprintf(LOG_DEBUG, "%s: Requested element is out of range\n",
		                   __func__);
		return NULL;
	}
	return (void*)(v->data + i * v->element_size);
}

int dynamic_array_size(const struct dynamic_array *v)
{
	if (!v) {
		lprintf(LOG_ERR, "%s: Invalid argument, v: %p\n", __func__, v);
		return -1;
	}
	return v->size;
}
