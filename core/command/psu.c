/*
 * Copyright 2014, Google Inc.
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

#include <stdio.h>

#include "mosys/kv_pair.h"
#include "mosys/platform.h"

const char *psu_type_names[] = {
	[PSU_TYPE_UNKNOWN]	= "unknown",
	[PSU_TYPE_BATTERY]	= "battery",	/* AC + rechargeable battery */
	[PSU_TYPE_AC_ONLY]	= "AC_only",	/* No battery in system */
};

static int psu_print_type(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	struct kv_pair *kv;
	enum psu_types type;

	/*
	 * Since mosys runs mostly on laptops currently, we'll just assume
	 * battery as a default unless there is a platform callback defined
	 * to tell us otherwise.
	 */
	if (!intf->cb->psu || !intf->cb->psu->type)
		type = PSU_TYPE_BATTERY;
	else
		type = intf->cb->psu->type(intf);

	kv = kv_pair_new();
	if (!kv)
		return -1;
	kv_pair_fmt(kv, "type", "%s", psu_type_names[type]);
	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

struct platform_cmd psu_cmds[] = {
	{
		.name	= "type",
		.desc	= "Print Power Supply Type",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = psu_print_type},
	},
	{ NULL }
};

struct platform_cmd cmd_psu = {
	.name	= "psu",
	.desc	= "Power Supply Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = psu_cmds }
};
