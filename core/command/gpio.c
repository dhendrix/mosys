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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "mosys/platform.h"
#include "mosys/log.h"

static int gpio_list_cmd(struct platform_intf *intf,
			 struct platform_cmd *cmd,
			 int argc, char **argv)
{
	if (!intf->cb->gpio || !intf->cb->gpio->list) {
		errno = ENOSYS;
		return -1;
	}

	return intf->cb->gpio->list(intf);
}

static int gpio_set_cmd(struct platform_intf *intf,
			struct platform_cmd *cmd,
			int argc, char **argv)
{
	int state;

	if (argc != 2) {
		platform_cmd_usage(cmd);
		errno = EINVAL;
		return -1;
	}

	if (!intf->cb->gpio || !intf->cb->gpio->set) {
		errno = ENOSYS;
		return -1;
	}

	state = atoi(argv[1]);
	if (state != 0 && state != 1) {
		platform_cmd_usage(cmd);
		errno = EINVAL;
		return -1;
	}

	return intf->cb->gpio->set(intf, argv[0], state);
}

struct platform_cmd gpio_cmds[] = {
	{
		.name	= "list",
		.desc	= "List GPIOs",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = gpio_list_cmd }
	},
	{
		.name	= "set",
		.desc	= "Set GPIO state",
		.usage	= "<name> <0|1>",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = gpio_set_cmd }
	},
	{ NULL }
};

struct platform_cmd cmd_gpio = {
	.name	= "gpio",
	.desc	= "GPIO Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = gpio_cmds }
};
