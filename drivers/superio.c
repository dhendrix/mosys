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
 *
 * superio.c: This is intended for generic SuperIO configuration functions.
 * Functions for a specific SuperIO or Embedded Controller (EC) chips should
 * go in a different driver.
 */

#include <inttypes.h>

#include "drivers/superio.h"

#include "intf/io.h"

#include "mosys/platform.h"

uint8_t sio_read(struct platform_intf *intf, uint16_t port, uint8_t reg)
{
	uint8_t val = 0;

	io_write8(intf, port, reg);
	io_read8(intf, port+1, &val);

	return val;
}

void sio_write(struct platform_intf *intf, uint16_t port,
               uint8_t reg, uint8_t data)
{
	io_write8(intf, port, reg);
	io_write8(intf, port+1, data);
}
