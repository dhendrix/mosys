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
	/* FIXME: historically we have only worried about SMBus devices here
	   (e.g. SPDs), but this does not seem like a great general solution */
	length = intf->op->i2c->smbus_read_reg(intf, bus, address,
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
