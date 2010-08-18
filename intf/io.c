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
 * io.c: IO register access interface
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>

#include "mosys/file_backed_range.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/string.h"

#include "intf/io.h"

inline struct file_backed_range *
io_ranges(struct platform_intf *intf)
{
	return intf->op->io->ranges;
}

/*
 * io_setup
 *
 * @intf:     platform interface
 *
 * returns 0 if io_ranges() is set to non-null
 * returns -1 if io_ranges() is NULL
 */
static int io_setup(struct platform_intf *intf)
{
	if (io_ranges(intf)) {
		return 0;
	}
	return -1;
}

/*
 * io_destroy
 *
 * @intf:     platform interface
 *
 */
static void io_destroy(struct platform_intf *intf)
{
	(void)intf;
	return;
}

/* open /dev/port */
static int
io_dev_open(struct platform_intf *intf, uint16_t reg,
            enum io_access_width size, int flags)
{
	struct file_backed_range *file_range;
	char *file_name;
	int fd;

	file_range = find_file_backed_range(reg, size, io_ranges(intf));
	if (file_range == NULL) {
		lprintf(LOG_DEBUG, "Unable to find backing file for range.\n");
		return -1;
	}

	file_name = format_string("%s/%s", mosys_get_root_prefix(),
	                          file_range->file_name);
	fd = open(file_name, flags);
	if (fd < 0) {
		lprintf(LOG_ERR, "Failed to open file %s\n", file_name);
		free(file_name);
		return -1;
	}
	free(file_name);

	return fd;
}

/*
 * io_read_dev  -  Read from IO register via /dev
 *
 * @intf:	platform interface
 * @reg:	register
 * @size:	number of bytes
 * @data:	pointer to data buffer
 */
static int io_read_dev(struct platform_intf *intf, uint16_t reg,
                       enum io_access_width size, void *data)
{
	int fd;
	int ret;

	fd = io_dev_open(intf, reg, size, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	ret = lseek(fd, reg, SEEK_SET);
	if (ret < 0) {
		close(fd);
		return -1;
	}

	switch (size) {
	  case IO_ACCESS_8:
	  case IO_ACCESS_16:
	  case IO_ACCESS_32:
		ret = read(fd, data, size);
		break;
	  default:
		ret = -1;
	}
	close(fd);

	return (ret == size) ?  0 : -1;
}

/*
 * io_write_dev  -  Write to IO register via /dev
 *
 * @intf:	platform interface
 * @reg:	register
 * @size:	number of bytes
 * @data:	pointer to data buffer
 */
static int io_write_dev(struct platform_intf *intf, uint16_t reg,
                        enum io_access_width size, void *data)
{
	int fd;
	int ret;

	fd = io_dev_open(intf, reg, size, O_RDWR);
	if (fd < 0) {
		return -1;
	}

	ret = lseek(fd, reg, SEEK_SET);
	if (ret < 0) {
		close(fd);
		return -1;
	}

	switch (size) {
	  case IO_ACCESS_8:
	  case IO_ACCESS_16:
	  case IO_ACCESS_32:
		ret = write(fd, data, size);
		break;
	  default:
		ret = -1;
	}
	close(fd);

	return (ret == size) ?  0 : -1;
}

#define IOPORT_DEV		"/dev/port"

static struct file_backed_range io_file_ranges[] = {
	/* 64K of IO ports */
	FILE_BACKED_RANGE_INIT(0, 0x10000, IOPORT_DEV),
	FILE_BACKED_RANGE_END,
};

/* Memory-mapped interface */
struct io_intf io_sys_intf = {
	.ranges		= io_file_ranges,
	.setup		= io_setup,
	.destroy	= io_destroy,
	.read		= io_read_dev,
	.write		= io_write_dev,
};
