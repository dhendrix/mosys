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
 * superio.h: This is intended for generic SuperIO configuration functions.
 * Functions for a specific SuperIO or Embedded Controller (EC) chips should
 * go in a different driver.
 */

#ifndef MOSYS_DRIVERS_SUPERIO_H__
#define MOSYS_DRIVERS_SUPERIO_H__

#include "mosys/platform.h"

/* common superio global config registers */
#define SIO_LDNSEL	0x07
#define SIO_CHIPID1	0x20
#define SIO_CHIPID2	0x21
#define SIO_CHIPVER	0x22
#define SIO_CTL		0x23

struct sio_id {
	char *vendor;
	char *name;
	uint8_t num_id_bytes;
	uint8_t chipid[2];
	uint8_t chipver;
};

/*
 * sio_read - read from SuperIO
 *
 * intf:	platform interface
 * port:	super io port
 * reg:		register offset within current logical device
 *
 * returns logical device number to indicate success
 * returns <0 to indicate failure
 */
extern uint8_t sio_read(struct platform_intf *intf, uint16_t port, uint8_t reg);

/*
 * sio_write - write to SuperIO
 *
 * intf:	platform interface
 * port:	super io port
 * reg:		register offset within current logical device
 * data:	data to write
 *
 * returns <0 to indicate failure
 */
extern void sio_write(struct platform_intf *intf, uint16_t port,
                      uint8_t reg, uint8_t data);

const struct sio_id *get_sio_id(struct platform_intf *intf, uint16_t port);

#endif	/* MOSYS_DRIVERS_SUPERIO_H__ */
