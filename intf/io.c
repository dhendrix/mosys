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
#include <sys/io.h>
#include <sys/stat.h>

#include "mosys/file_backed_range.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/string.h"

#include "intf/io.h"

#define IOPORT_DEV		"/dev/port"

static struct io_intf io_raw_intf;
static struct io_intf io_sys_intf;

static int setup_done;

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
 * returns 0 if setup is successful
 * returns -1 to indicate failure
 */
static int io_setup(struct platform_intf *intf)
{
	struct stat s;
	int ret = 0;

	if (stat(IOPORT_DEV, &s) < 0) {
		lprintf(LOG_DEBUG, "%s: using raw port IO\n", __func__);
		intf->op->io = &io_raw_intf;
#if defined(PLATFORM_ARCH_X86)
		if (iopl(3) != 0) {
			lprintf(LOG_ERR, "%s: cannot set IO permissions\n",
			                 __func__);
			ret = -1;
		}
#endif
	} else {
		lprintf(LOG_DEBUG, "%s: using file-backed port IO\n", __func__);
		intf->op->io = &io_sys_intf;
		if (!io_ranges(intf)) {
			ret = -1;
		}
	}

	setup_done = 1;
	return ret;
}

/*
 * io_destroy_dev - destroy interface (for /dev)
 *
 * @intf:     platform interface
 *
 */
static void io_destroy_dev(struct platform_intf *intf)
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

	if (!setup_done)
		intf->op->io->setup(intf);

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

	if (!setup_done)
		intf->op->io->setup(intf);

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

static struct file_backed_range io_file_ranges[] = {
	/* 64K of IO ports */
	FILE_BACKED_RANGE_INIT(0, 0x10000, IOPORT_DEV),
	FILE_BACKED_RANGE_END,
};

/* Memory-mapped interface */
static struct io_intf io_sys_intf = {
	.ranges		= io_file_ranges,
	.setup		= io_setup,
	.destroy	= io_destroy_dev,
	.read		= io_read_dev,
	.write		= io_write_dev,
};

/*
 * io_destroy_raw - destroy interface (for raw access)
 *
 * @intf:     platform interface
 *
 */
static void io_destroy_raw(struct platform_intf *intf)
{
	(void)intf;
	return;
}

/*
 * io_read_raw  -  Read from IO register using inb/inw/inl
 *
 * @intf:	platform interface
 * @reg:	register
 * @size:	number of bytes
 * @data:	pointer to data buffer
 */
static int io_read_raw(struct platform_intf *intf, uint16_t reg,
                       enum io_access_width size, void *data)
{
	if (!setup_done)
		intf->op->io->setup(intf);

	switch(size) {
	case IO_ACCESS_8: {
		uint8_t value = inb(reg);
		memcpy(data, &value, size);
		break;
	}
	case IO_ACCESS_16: {
		uint16_t value = inw(reg);
		memcpy(data, &value, size);
		break;
	}
	case IO_ACCESS_32: {
		uint32_t value = inl(reg);
		memcpy(data, &value, size);
		break;
	}
	default:
		lprintf(LOG_DEBUG, "invalid io access width: %d\n", size);
		return -1;
	}

	return 0;
}

/*
 * io_write_raw  -  Write to IO register using outb/outw/outl
 *
 * @intf:	platform interface
 * @reg:	register
 * @size:	number of bytes
 * @data:	pointer to data buffer
 */
static int io_write_raw(struct platform_intf *intf, uint16_t reg,
                        enum io_access_width size, void *data)
{
	int ret = 0;

	if (!setup_done)
		intf->op->io->setup(intf);

	switch (size) {
	case IO_ACCESS_8:
		outb(*(uint8_t *)data, reg);
		break;
	case IO_ACCESS_16:
		outw(*(uint16_t *)data, reg);
		break;
	case IO_ACCESS_32:
		outl(*(uint32_t *)data, reg);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static struct io_intf io_raw_intf = {
	.setup		= io_setup,
	.destroy	= io_destroy_raw,
	.read		= io_read_raw,
	.write		= io_write_raw,
};

struct io_intf io_intf = {
	.setup		= io_setup,
};
