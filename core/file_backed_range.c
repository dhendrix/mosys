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
 * Support routines for struct file_backed_range.
 */

#include <stddef.h>
#include "mosys/file_backed_range.h"

struct file_backed_range *
find_file_backed_range(uint64_t address, size_t length,
                       struct file_backed_range *ranges)
{
	struct file_backed_range *file_range;
	uint64_t end_address = address + length - 1;

	file_range = ranges;

	while (file_range->file_name) {
		if (address_in_range(&file_range->range, address) &&
		    address_in_range(&file_range->range, end_address)) {
			return file_range;
		}
		file_range++;
	}

	return NULL;
}
