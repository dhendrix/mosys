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
