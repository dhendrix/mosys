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

#ifndef MOSYS_LIB_DYNAMIC_ARRAY_H__
#define MOSYS_LIB_DYNAMIC_ARRAY_H__

#include <sys/types.h>

#define DEFAULT_DYNAMIC_ARRAY_CAPACITY 10

struct dynamic_array;

/* Creates a new dynamic array instance of a default size.  */
extern struct dynamic_array *new_dynamic_array(size_t element_size);

/*
 * Frees a dynamic array structure.
 *
 * This is a null operation if v is NULL.
 *
 * @v: target dynamic_array instance.
 */
extern void free_dynamic_array(struct dynamic_array *v);

/*
 * dynamic_array_push_back - Pushes data element to the end of dynamic_array
 *
 * @v: target dynamic_array instance
 * @data: pointer to data to store in the dynamic_array
 *
 * returns 0 to indicate success 
 * returns <0 to indicate error
 */
extern int dynamic_array_push_back(struct dynamic_array *v, void *data);

/*
 * dynamic_array_get - Retrieves an element from a dynamic_array instance.
 *
 * @v: target dynamic_array instance
 * @i: index of target element
 *
 * Returns a pointer to the requested element
 * Returns NULL to indicate failure
 */
extern void *dynamic_array_get(const struct dynamic_array *v, size_t i);

/*
 * Returns the specified entry based on the given type
 */
#define dynamic_array_get_entry(dyn_arr_, entry_num_, type_) \
	*(type_ *)dynamic_array_get((dyn_arr_), (entry_num_))

/*
 * dynamic_array_size - number of elements in the dynamic_array instance.
 *
 * @v: target dynamic_array instance
 *
 * Returns the number of elements in the dynamic_array instance
 * Returns <0 to indicate failure
 */
extern int dynamic_array_size(const struct dynamic_array *v);

/* for unit testing */
extern int dynamic_array_unittest(void);

#endif	/* MOSYS_LIB_DYNAMIC_ARRAY_H__ */
