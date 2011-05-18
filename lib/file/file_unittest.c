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

#include "mosys/globals.h"
#include "mosys/platform.h"

#include "lib/file.h"
#include "mosys/list.h"

static void scanft_test(void **state) {
	struct ll_node *list = NULL;
	char root[PATH_MAX];

	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "scanft_test/");

	/* The first needle should be in given root. */
	assert_true(scanft(&list, root, "needle0", NULL, 0) != NULL);
	list_cleanup(&list);

	/* The second needle requires some basic recursion to find. */
	assert_true(scanft(&list, root, "needle1", NULL, 1) != NULL);
	list_cleanup(&list);

	/* The third needle should requires recursion and symlink
	 * handling to find. */
	assert_true(scanft(&list, root, "needle2", NULL, 1) != NULL);
	list_cleanup(&list);

	/*
	 * The fourth needle doesn't exist so scanft() should return NULL. A
	 * self-referencing directory symlink has been added to the hierarchy to
	 * test symlink depth handling.
	 */
	assert_true(scanft(&list, root, "needle3", NULL, 100) == NULL);
	list_cleanup(&list);
}

static void sysfs_lowest_smbus_test(void **state) {
	char root[PATH_MAX];

	/*
	 * There are three haystacks, each with a needle.
	 *
	 * test1/ : needle is i2c-0, hay is in i2c-1 and i2c-2
	 * test2/ : needle is i2c-1, hay is in i2c-0 and i2c-2
	 * test3/ : needle is i2c-2, hay is in i2c-0 and i2c-1
	 */
	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "sysfs_lowest_smbus_test/test1/");
	/* FIXME: for debugging */
	printf("root: %s\n", root);
	assert_int_equal(0, sysfs_lowest_smbus(root, "needle"));

#if 0
	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "sysfs_lowest_smbus_test/test2");
	assert_int_equal(1, sysfs_lowest_smbus(root, "needle"));

	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "sysfs_lowest_smbus_test/test3");
	assert_int_equal(2, sysfs_lowest_smbus(root, "needle"));

	/* test invalid path */
	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "sysfs_lowest_smbus_test/nothing");
	assert_int_equal(-1, sysfs_lowest_smbus(root, "needle"));
#endif
}

int file_unittest(struct platform_intf *intf)
{
	UnitTest tests[] = {
//		unit_test(scanft_test),
		unit_test(sysfs_lowest_smbus_test),
	};

	return run_tests(tests);
}
