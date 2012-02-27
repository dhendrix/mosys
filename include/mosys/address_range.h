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
 * Structures and support routines for tracking memory ranges.
 */

#ifndef MOSYS_ADDRESS_RANGE_H__
#define MOSYS_ADDRESS_RANGE_H__

#include <stdint.h>

/* a helper structure and functions for tracking memory ranges */
struct address_range
{
	/* begin and end are inclusive in the range */
	uint64_t begin;
	uint64_t end;
};

#define ADDRESS_RANGE_INIT(begin_, end_) \
	{ (begin_), (end_) }

static inline uint64_t
address_range_size(const struct address_range *range)
{ // COV_NF_LINE
	/* Note: the address_range cannot deliver a size of 2^64 (==0). */
	return range->end - range->begin + 1;
}

static inline int
address_in_range(const struct address_range *range, uint64_t addr)
{ // COV_NF_LINE
	return (addr >= range->begin && addr <= range->end);
}

/* Set the inclusive lower bounds for a given memory range.  */
static inline void
set_address_range_begin(struct address_range *range, uint64_t addr)
{ // COV_NF_LINE
	range->begin = addr;
}

static inline uint64_t
get_address_range_begin(struct address_range *range)
{ // COV_NF_LINE
	return range->begin;
}

/* Set the inclusive upper bounds for a given memory range. */
static inline void
set_address_range_end(struct address_range *range, uint64_t end)
{ // COV_NF_LINE
	range->end = end;
}

static inline uint64_t
get_address_range_end(const struct address_range *range)
{ // COV_NF_LINE
	return range->end;
}

#endif /* MOSYS_ADDRESS_RANGE_H__ */
