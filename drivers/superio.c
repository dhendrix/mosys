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

#include "lib/math.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

static struct sio_id superio_ids[] = {
	{ "ite", "it8500", 2, { 0x85, 0x00 } },
	{ "ite", "it8772", 2, { 0x87, 0x72 } },
	{ "nuvoton", "npce781", 1, { 0xfc, 0x00 } },
	{ "smsc", "mec1308", 2, { 0x2d, 0x00 } },
	{ "smsc", "mec1308", 2, { 0x4d, 0x01 } },
	{ "smsc", "mec1310", 2, { 0x04, 0x02 } },
};

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

const struct sio_id *get_sio_id(struct platform_intf *intf, uint16_t port)
{
	static struct sio_id *id = NULL;
	uint8_t id_byte[2];
	static unsigned short done = 0;
	int i;
	unsigned short found = 0;

	if (done)
		return id;
	lprintf(LOG_DEBUG, "%s: using port 0x%04x\n", __func__, port);
	id_byte[0] = sio_read(intf, port, SIO_CHIPID1);
	id_byte[1] = sio_read(intf, port, SIO_CHIPID2);

	for (i = 0; i < ARRAY_SIZE(superio_ids); i++) {
		lprintf(LOG_DEBUG, "%s: comparing 0x%02x%02x and 0x%02x%02x\n",
				   __func__,
				   superio_ids[i].chipid[0],
				   superio_ids[i].chipid[1],
				   id_byte[0], id_byte[1]);
		if (superio_ids[i].num_id_bytes == 1) {
			if (superio_ids[i].chipid[0] == id_byte[0]) {
				found = 1;
				break;
			}
		} else if (superio_ids[i].num_id_bytes == 2) {
			if (superio_ids[i].chipid[0] == id_byte[0] &&
			    superio_ids[i].chipid[1] == id_byte[1]) {
				found = 1;
				break;
			}
		}
	}

	if (found) {
		id = &superio_ids[i];
		id->chipver = sio_read(intf, port, SIO_CHIPVER);
	}

	done = 1;
	return id;
}
