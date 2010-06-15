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
