/*
 * Copyright (C) 2011 Google Inc.
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

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "cmockery.h"

#include "mosys/file_backed_range.h"
#include "mosys/platform.h"

#include "mosys/globals.h"

#include "intf/io.h"


#define IO_DATA_PATH		"io/"	/* relative to the test data prefix */

static struct platform_intf *intf;

/* Test a range with an address out of bounds */
static void bad_address(void **state)
{
	uint8_t data;
	static struct file_backed_range out_of_bounds_file_ranges[] = {
		FILE_BACKED_RANGE_INIT(0, 0x10, "/dev/port"),
		FILE_BACKED_RANGE_END
	};
	struct file_backed_range *saved_file_ranges = intf->op->io->ranges;

	intf->op->io->ranges = out_of_bounds_file_ranges;
	assert_int_equal(0, intf->op->io->setup(intf));

	assert_int_equal(-1,
		      intf->op->io->read(intf, 0x20, IO_ACCESS_8, &data));

	assert_int_equal(-1,
		      intf->op->io->write(intf, 0x20, IO_ACCESS_8, &data));

	/* restore the original value */
	intf->op->io->ranges = saved_file_ranges;
	intf->op->io->destroy(intf);
}

/* Test a non-existing dev file */
static void open_nonexistent_file(void **state)
{
	uint8_t data;
	static struct file_backed_range nonexistent_file_ranges[] = {
		FILE_BACKED_RANGE_INIT(0, 0x10000, "nonexistent"),
		FILE_BACKED_RANGE_END
	};
	struct file_backed_range *saved_file_ranges = intf->op->io->ranges;

	intf->op->io->ranges = nonexistent_file_ranges;
	assert_int_equal(0, intf->op->io->setup(intf));

	assert_int_equal(-1, intf->op->io->read(intf, 0, IO_ACCESS_8, &data));

	intf->op->io->ranges = saved_file_ranges;
	intf->op->io->destroy(intf);
}

/* Test a bad read by having a range larger than the actual dev file */
static void read_eof(void **state)
{
	uint8_t data;
	static struct file_backed_range oversize_file_ranges[] = {
		FILE_BACKED_RANGE_INIT(0, 0x10000, "/dev/port"),
		FILE_BACKED_RANGE_END
	};
	struct file_backed_range *saved_file_ranges = intf->op->io->ranges;

	intf->op->io->ranges = oversize_file_ranges;
	assert_int_equal(0, intf->op->io->setup(intf));

	assert_int_equal(-1, intf->op->io->read(intf, 0xffff,
	                                        IO_ACCESS_8, &data));

	/* restore the original value */
	intf->op->io->ranges = saved_file_ranges;
	intf->op->io->destroy(intf);
}

static void io_read_test(void **state)
{
	int ret;
	uint8_t data8;
	uint16_t data16;
	uint32_t data32;

	/* 8 bit read */
	data8 = 0xff;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	assert_int_equal(0x00, (int) data8);
	/* now through inlines */
	data8 = 0xff;
	ret = io_read(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	assert_int_equal(0x00, (int) data8);
	data8 = 0xff;
	ret = io_read8(intf, 0, &data8);
	assert_int_equal(0, ret);
	assert_int_equal(0x00, (int) data8);

	/* 16 bit read */
	data16 = 0xffff;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_16, &data16);
	assert_int_equal(0, ret);
	assert_int_equal(0x0100, data16);
	/* now through inlines */
	data16 = 0xffff;
	ret = io_read16(intf, 0, &data16);
	assert_int_equal(0, ret);
	assert_int_equal(0x0100, data16);

	/* 32 bit read */
	data32 = 0xffffffff;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_32, &data32);
	assert_int_equal(0, ret);
	assert_int_equal(0x03020100, data32);
	/* now through inlines */
	data32 = 0xffffffff;
	ret = io_read32(intf, 0, &data32);
	assert_int_equal(0, ret);
	assert_int_equal(0x03020100, data32);
}

static void io_write_test(void **state)
{
	int ret;
	uint8_t data8, orig8;
	uint16_t data16, orig16;
	uint32_t data32, orig32;

	/* 8 bit write */
	data8 = 0xff;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_8, &orig8);
	assert_int_equal(0, ret);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	data8 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	assert_int_equal(0xff, (int) data8);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_8, &orig8);
	assert_int_equal(0, ret);
	/* now through inlines */
	data8 = 0xff;
	ret = io_write(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	data8 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	assert_int_equal(0xff, (int) data8);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_8, &orig8);
	assert_int_equal(0, ret);
	data8 = 0xff;
	ret = io_write8(intf, 0, data8);
	assert_int_equal(0, ret);
	data8 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_8, &data8);
	assert_int_equal(0, ret);
	assert_int_equal(0xff, (int) data8);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_8, &orig8);
	assert_int_equal(0, ret);

	/* 16 bit write */
	data16 = 0xffff;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_16, &orig16);
	assert_int_equal(0, ret);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_16, &data16);
	assert_int_equal(0, ret);
	data16 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_16, &data16);
	assert_int_equal(0, ret);
	assert_int_equal(0xffff, data16);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_16, &orig16);
	assert_int_equal(0, ret);
	/* now through inlines */
	data16 = 0xffff;
	ret = io_write16(intf, 0, data16);
	assert_int_equal(0, ret);
	data16 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_16, &data16);
	assert_int_equal(0, ret);
	assert_int_equal(0xffff, data16);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_16, &orig16);
	assert_int_equal(0, ret);

	/* 32 bit write */
	data32 = 0xffffffff;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_32, &orig32);
	assert_int_equal(0, ret);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_32, &data32);
	assert_int_equal(0, ret);
	data32 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_32, &data32);
	assert_int_equal(0, ret);
	assert_int_equal(0xffffffff, data32);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_32, &orig32);
	assert_int_equal(0, ret);
	/* now through inlines */
	data32 = 0xffffffff;
	ret = io_write32(intf, 0, data32);
	assert_int_equal(0, ret);
	data32 = 0;
	ret = intf->op->io->read(intf, 0, IO_ACCESS_32, &data32);
	assert_int_equal(0, ret);
	assert_int_equal(0xffffffff, data32);
	ret = intf->op->io->write(intf, 0, IO_ACCESS_32, &orig32);
	assert_int_equal(0, ret);
}

int io_unittest(struct platform_intf *_intf)
{
	int ret;
	char root_prefix_for_test[PATH_MAX];
	char *root_prefix_orig;
	UnitTest tests[] = {
		unit_test(bad_address),
		unit_test(open_nonexistent_file),
		unit_test(read_eof),
		unit_test(io_read_test),
		unit_test(io_write_test),
	};

	intf = _intf;
	intf->op->io = &io_intf;

	root_prefix_orig = strdup(mosys_get_root_prefix());
	sprintf(root_prefix_for_test, "%s/%s", root_prefix_orig, IO_DATA_PATH);
	printf("data root prefix: %s\n", root_prefix_for_test);
	mosys_set_root_prefix(root_prefix_for_test);

	intf->op->io->setup(intf);
	free(root_prefix_orig);
	ret = run_tests(tests);
	mosys_set_root_prefix(root_prefix_orig);

	return ret;
}
