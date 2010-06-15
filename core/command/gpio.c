/*
 * Copyright 2007 Google Inc. All Rights Reserved.
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
#include "mosys/common.h"
#include "mosys/log.h"

static int gpio_list_cmd(struct platform_intf *intf,
			 struct platform_cmd *cmd,
			 int argc, char **argv)
{
	if (!intf->cb->gpio || !intf->cb->gpio->list) {
		return -ENOSYS;
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
		return -1;
	}

	if (!intf->cb->gpio || !intf->cb->gpio->set) {
		return -ENOSYS;
	}

	state = atoi(argv[1]);
	if (state != 0 && state != 1) {
		platform_cmd_usage(cmd);
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
