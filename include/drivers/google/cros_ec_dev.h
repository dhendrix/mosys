/*
 * Copyright (C) 2013 The Chromium OS Authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CROS_EC_DEV_H__
#define CROS_EC_DEV_H__

#include <linux/types.h>

/*
 * @version: Command version number (often 0)
 * @command: Command to send (EC_CMD_...)
 * @outdata: Outgoing data to EC
 * @outsize: Outgoing length in bytes
 * @indata: Where to put the incoming data from EC
 * @insize: Incoming length in bytes (filled in by EC)
 * @result: EC's response to the command (separate from communication failure)
 */
struct cros_ec_command {
	uint32_t version;
	uint32_t command;
	const uint8_t *outdata;
	uint32_t outsize;
	uint8_t *indata;
	uint32_t insize;
	uint32_t result;
};

#define CROS_EC_DEV_IOC		':'
#define CROS_EC_DEV_IOCXCMD	_IOWR(':', 0, struct cros_ec_command)

#endif /* CROS_EC_DEV_H__ */
