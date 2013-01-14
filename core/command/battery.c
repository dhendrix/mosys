/*
 * Copyright 2012, Google Inc.
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

#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

static int battery_print_fud_cmd(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	const char *date;
	struct kv_pair *kv;

	if (!intf->cb->battery || !intf->cb->battery->get_fud)
		return -ENOSYS;

	date = intf->cb->battery->get_fud(intf);
	if (!date)
		return -EIO;

	kv = kv_pair_new();
	kv_pair_add(kv, "first_use_date", date);
	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int battery_set_fud_cmd(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	time_t t;
	struct tm *system_time;

	if (!intf->cb->battery || !intf->cb->battery->set_fud)
		return -ENOSYS;

	t = time(0);
	system_time = localtime(&t);

	return intf->cb->battery->set_fud(intf,
					  system_time->tm_mday,
                                          system_time->tm_mon + 1,
                                          system_time->tm_year + 1900);
}


static int battery_update_cmd(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	if (!intf->cb->battery || !intf->cb->battery->update)
		return -ENOSYS;

	return intf->cb->battery->update(intf);
}

struct platform_cmd battery_print_cmds[] = {
	{
		.name	= "fud",
		.desc	= "Print Battery First Use Date",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = battery_print_fud_cmd },
	},
	{ NULL }
};

struct platform_cmd battery_set_cmds[] = {
	{
		.name	= "fud",
		.desc	= "Set Battery First Use Date",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = battery_set_fud_cmd },
	},
	{ NULL }
};

struct platform_cmd battery_cmds[] = {
	{
		.name	= "print",
		.desc	= "Print Battery Info",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = battery_print_cmds },
	},
	{
		.name	= "set",
		.desc	= "Set Battery Variables",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = battery_set_cmds },
	},
	{
		.name	= "update",
		.desc	= "Initiate Battery Firmware Update",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .sub = battery_update_cmd },
	},
	{ NULL }
};

struct platform_cmd cmd_battery = {
	.name	= "battery",
	.desc	= "Battery Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = battery_cmds }
};
