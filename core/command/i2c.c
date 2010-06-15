/*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "mosys/platform.h"
#include "mosys/output.h"
#include "mosys/log.h"

#include "intf/i2c.h"

static int i2c_dump_cmd(struct platform_intf *intf,
			struct platform_cmd *cmd, int argc, char **argv)
{
	uint8_t i2c_data[256];
	int bus, address;
	int start = 0;
	int length = 256;

	/* get bus and address from command line */
	if (argc < 2) {
		platform_cmd_usage(cmd);
		return -1;
	}
	bus = (int)strtol(argv[0], NULL, 0);
	address = (int)strtol(argv[1], NULL, 0);

	if (argc == 3) {
		length = (int)strtol(argv[2], NULL, 0);
	}
	if (argc == 4) {
		start = (int)strtol(argv[2], NULL, 0);
		length = (int)strtol(argv[3], NULL, 0);
	}

	lprintf(LOG_DEBUG, "i2c dump %d %d [reg %d-%d]\n",
		bus, address, start, length);

	/* read in block of 256 bytes */
	memset(i2c_data, 0, sizeof(i2c_data));
	length = intf->op->i2c->read_reg(intf, bus, address,
					 start, length, i2c_data);

	if (length < 0) {
		lprintf(LOG_ERR, "Failed to read from I2C (not present?)\n");
		return -1;
	}

	print_buffer(i2c_data, length);

	return 0;
}

struct platform_cmd i2c_cmds[] = {
	{
		.name	= "dump",
		.desc	= "Dump from I2C device",
		.usage	= "<bus> <address> [start] [length]",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = i2c_dump_cmd }
	},
	{ NULL }
};

struct platform_cmd cmd_i2c = {
	.name	= "i2c",
	.desc	= "I2C Commands",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = i2c_cmds }
};
