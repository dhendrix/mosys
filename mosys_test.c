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

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>

#include "cmockery.h"

#include "mosys/log.h"
#include "mosys/platform.h"

const char *test_ids[] = {
	"TEST",
	NULL,
};

struct platform_op test_ops;
struct platform_cb test_cbs;

struct platform_intf test_intf = {
	.type 		= PLATFORM_DEFAULT,
	.name 		= "UNITTEST",
	.id_list 	= test_ids,
	.op		= &test_ops,
	.cb		= &test_cbs,
};

int main(int argc, char **argv)
{
	int rc = 0;
	struct platform_intf *intf = &test_intf;

	mosys_globals_init();
	mosys_set_root_prefix(UNITTEST_DATA);

	rc |= file_unittest(intf);
	rc |= math_unittest();
	rc |= io_unittest(intf);

	if (rc == 0)
		fprintf(stdout, "Unit tests passed.\n");
	else
		fprintf(stdout, "Unit tests failed.\n");

	return rc;
}
