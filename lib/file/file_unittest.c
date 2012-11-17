/* Copyright 2012, Google Inc.
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
 */

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmockery.h"

#include "mosys/globals.h"
#include "mosys/list.h"
#include "mosys/platform.h"

#include "lib/file.h"

static void scanft_test(void **state) {
	struct ll_node *list = NULL;
	char root[PATH_MAX];

	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "scanft_test/");

	/* The first needle should be in given root. */
	assert_true(scanft(&list, root, "needle0", NULL, -1, 0) != NULL);
	list_cleanup(&list);

	/* The second needle requires some basic recursion to find. */
	assert_true(scanft(&list, root, "needle1", NULL,, -1, 1) != NULL);
	list_cleanup(&list);

	/* The third needle should requires recursion and symlink
	 * handling to find. */
	assert_true(scanft(&list, root, "needle2", NULL, -1, 1) != NULL);
	list_cleanup(&list);

	/*
	 * The fourth needle doesn't exist so scanft() should return NULL. A
	 * self-referencing directory symlink has been added to the hierarchy to
	 * test symlink depth handling.
	 */
	assert_true(scanft(&list, root, "needle3", NULL, -1, 100) == NULL);
	list_cleanup(&list);
}

static void sysfs_lowest_smbus_test(void **state) {
	char root[PATH_MAX];

	/*
	 * Find "needle" in haystack of sysfs-style i2c-<bus> directories.
	 *
	 * test0/ : no needles, just hay
	 * test1/ : needle is i2c-0, hay is in i2c-1 and i2c-2
	 * test2/ : needle is i2c-1, hay is in i2c-0 and i2c-2
	 * test3/ : needle is i2c-2, hay is in i2c-0 and i2c-1
	 * test4/ : needles are in i2c-0 and i2c-2, hay is in i2c-1
	 */
	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "sysfs_lowest_smbus_test/test0/");
	assert_int_equal(-1, sysfs_lowest_smbus(root, "needle"));

	snprintf(root, sizeof(root), "%s/%s",
	         mosys_get_root_prefix(), "sysfs_lowest_smbus_test/test1/");
	assert_int_equal(0, sysfs_lowest_smbus(root, "needle"));

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
}

int file_unittest(struct platform_intf *intf)
{
	UnitTest tests[] = {
		unit_test(scanft_test),
		unit_test(sysfs_lowest_smbus_test),
	};

	return run_tests(tests);
}
