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
 * A file_backed_range is a simple abstraction for mapping ranges of
 * address space to a file.  The obvious usage is in doing memory mapped
 * IO via /dev/mem, but it can be used in other places, too.
 */

#ifndef MOSYS_FILE_BACKED_RANGE_H__
#define MOSYS_FILE_BACKED_RANGE_H__

#include <stdint.h>
#include "mosys/address_range.h"

/* a helper struct and functions for mapping ranges to files */
struct file_backed_range {
	struct address_range range;
	const char *file_name;
};

#define FILE_BACKED_RANGE_INIT(begin_, size_, file_) \
	{ \
		ADDRESS_RANGE_INIT(begin_, (begin_) + (size_) - 1ULL), \
		file_, \
	}

#define FILE_BACKED_RANGE_END \
	{ \
		ADDRESS_RANGE_INIT(0, 0), \
		NULL, \
	}

/*
 * Given an array of file_backed_range structures, find the one that
 * includes the specified address.
 *
 * @address:  address of memory to map
 * @length:   length of data to map
 * @ranges:   array of file_backed_range
 */
extern struct file_backed_range *
find_file_backed_range(uint64_t address,
                       size_t length, struct file_backed_range *ranges);

#endif /* MOSYS_FILE_BACKED_RANGE_H__ */
