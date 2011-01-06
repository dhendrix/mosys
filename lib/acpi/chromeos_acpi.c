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
 */

#include <stdlib.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/log.h"

#include "lib/acpi.h"
#include "lib/file.h"

int acpi_get_hwid(char **buf)
{
	char path[] = CHROMEOS_ACPI_PATH"HWID";
	int fd, len = -1;

	fd = file_open(path, FILE_READ);
	if (fd < 0)
		return -1;

	*buf = mosys_malloc(CHROMEOS_HWID_MAXLEN);
	memset(*buf, 0, CHROMEOS_HWID_MAXLEN);
	len = read(fd, *buf, CHROMEOS_HWID_MAXLEN);
	if (len < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read hwid from %s\n",
		                   __func__, path);
		free(*buf);
	}

	close(fd);
	return len;
}
