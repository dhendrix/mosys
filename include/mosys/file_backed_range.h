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
