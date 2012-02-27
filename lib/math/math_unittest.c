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
	assert_int_equal(5, __abs(5 - 10));
	assert_int_equal(5, __abs(-2 + -3));
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
