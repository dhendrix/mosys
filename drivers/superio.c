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
 * superio.c: This is intended for generic SuperIO configuration functions.
 * Functions for a specific SuperIO or Embedded Controller (EC) chips should
 * go in a different driver.
 */

#include <inttypes.h>
#include <sys/time.h>

#include "drivers/superio.h"

#include "intf/io.h"

#include "lib/math.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

static struct sio_id superio_ids[] = {
	{ "ite", "it8500", 2, { 0x85, 0x00 } },
	{ "ite", "it8518", 2, { 0x85, 0x18 } },
	{ "ite", "it8772", 2, { 0x87, 0x72 } },
	{ "nuvoton", "npce781", 1, { 0xfc, 0x00 } },
	{ "smsc", "mec1308", 2, { 0x2d, 0x00 } },
	{ "smsc", "mec1308", 2, { 0x4d, 0x01 } },
	{ "smsc", "mec1310", 2, { 0x04, 0x02 } },
};

static int i8042_ibf_clear(uint8_t status)
{
	return !(status & I8042_IBF);
}

static int i8042_obf_set(uint8_t status)
{
	return status & I8042_OBF;
}

static int i8042_poll_csr(struct platform_intf *intf,
			  struct i8042_host_intf *i8042_io,
			  unsigned int timeout_ms,
			  int (*cmp)(uint8_t status))
{
	uint8_t status;
	uint16_t csr;
	struct timeval begin, now;
	unsigned int timeout_s = timeout_ms / 1000;
	unsigned long int remainder_us = (timeout_ms % 1000) * 1000;

	if (i8042_io)
		csr = i8042_io->csr;
	else
		csr = I8042_CSR;

	gettimeofday(&begin, NULL);
	while (1) {
		if (io_read8(intf, csr, &status) < 0)
			return -1;

		if (cmp(status))
			return 1;

		gettimeofday(&now, NULL);
		if (now.tv_sec - begin.tv_sec >= timeout_s) {
			if (__abs(now.tv_usec - begin.tv_usec) >= remainder_us)
				break;
		}
	}

	return 0;
}

int i8042_wait_ibf_clear(struct platform_intf *intf,
		struct i8042_host_intf *i8042_io, unsigned int timeout_ms)
{
	return i8042_poll_csr(intf, i8042_io, timeout_ms, i8042_ibf_clear);
}

int i8042_wait_obf_set(struct platform_intf *intf,
		struct i8042_host_intf *i8042_io, unsigned int timeout_ms)
{
	return i8042_poll_csr(intf, i8042_io, timeout_ms, i8042_obf_set);
}

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
