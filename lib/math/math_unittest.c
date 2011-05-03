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

#include "mosys/platform.h"

#include "lib/math.h"

static void ctz_test(void **state)
{
	unsigned long long int u;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;

	/* Test -1, 0, 1 as unsigned long long ints and some numbers of
	 * common sizes */
	u = -1;
	assert_int_equal(0, ctz(u));
	u = 0;	/* The corner case for the "cast to float" algorithm */
	assert_int_equal(0, ctz(u));
	u = 1;
	assert_int_equal(0, ctz(u));

	u8 = 0xF0;
	u16 = 0xFF00;
	u32 = 0xFFFF0000;
	u64 = 0xFFFFFFFF00000000ULL;
	assert_int_equal(4, ctz(u8));
	assert_int_equal(8, ctz(u16));
	assert_int_equal(16, ctz(u32));
	assert_int_equal(32, ctz(u64));
}

static void logbase2_test(void **state)
{
	int i;

	/* Corner case, returns 0 */
	assert_int_equal(logbase2(0), 0);

	/* Test results for perfect powers of 2, positive and negative */
	for (i = 0; i < sizeof(i) * CHAR_BIT; i++) {
		assert_int_equal(i, logbase2(1 << i));
		assert_int_equal(i, logbase2(-(1 << i)));
	}

	/* Test non-power-of-2 numbers */
	for (i = 2; i < sizeof(i) * CHAR_BIT; i++) {
		assert_int_equal(i, logbase2((1 << i) + 1));
	}
}

static void rolling8_csum_test(void **state)
{
	size_t len = 256;
	uint8_t buf[len];
	int i;

	memset(buf, 0, len);

	for (i = 0; i < len; i++) {
		uint8_t expected = i + 1;
		buf[i] = 1;

		assert_int_equal(expected, rolling8_csum(buf, len));
	}
}

static void macro_unittest(void **state)
{
	int i;

	/* Test __mask() the 0 edge case */
	assert_int_equal(1, __mask(0, 0));

	/* Test __mask() with powers of 2 */
	for (i = 1; i < (sizeof(long long) * CHAR_BIT); i++) {
		unsigned long long mask = __mask((i - 1), 0);
		assert_int_equal((1ULL << i) - 1, mask);
	}

	/* Test __mask with non-zero bases */
	for (i = 0; i < ((sizeof(long long) * CHAR_BIT) - 8); i++) {
		unsigned long long mask = __mask(i + 8 - 1, i);
		assert_int_equal(0xffULL << i, mask);
	}

	/* test __min() macro with different semantics */
	assert_int_equal(0, __min(0, 1));
	assert_int_equal(0, __min(1, 0));
	assert_int_equal(-1, __min(-1, 0));
	assert_int_equal(-1, __min(0, -1));

	/* test __max() macro with different semantics */
	assert_int_equal(1, __max(0, 1));
	assert_int_equal(1, __max(1, 0));
	assert_int_equal(0, __max(-1, 0));
	assert_int_equal(0, __max(0, -1));

	/* test __abs() */
	assert_int_equal(0, __abs(0));
	assert_int_equal(5, __abs(5));
	assert_int_equal(5, __abs(-5));
}

int math_unittest(void)
{
	UnitTest tests[] = {
		unit_test(ctz_test),
		unit_test(logbase2_test),
		unit_test(rolling8_csum_test),
		unit_test(macro_unittest),
	};

	return run_tests(tests);
}
