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
 * Note: This file shares some code with the Flashrom project.
 *
 * mec1308: common routines for mec1308
 */

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "lib/math.h"

#include "drivers/superio.h"
#include "drivers/smsc/mec1308.h"

#define MEC1308_SIO_ENTRY_KEY		0x55
#define MEC1308_SIO_EXIT_KEY		0xaa

#define MEC1308_DEFAULT_SIO_PORT	0x2e
#define MEC1308_DEFAULT_MBX_IOBAD	0xa00

static unsigned int in_sio_cfgmode;

/* returns 0 to indicate success, <0 otherwise */
void mec1308_sio_enter(struct platform_intf *intf, uint16_t port)
{
	if (in_sio_cfgmode)
		return;

	io_write8(intf, port, MEC1308_SIO_ENTRY_KEY);
	in_sio_cfgmode = 1;
}

void mec1308_sio_exit(struct platform_intf *intf, uint16_t port)
{
	if (!in_sio_cfgmode)
		return;

	io_write8(intf, port, MEC1308_SIO_EXIT_KEY);
	in_sio_cfgmode = 0;
}

/*
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
int mec1308_get_sioport(struct platform_intf *intf, uint16_t *port)
{
	uint8_t ports[] = { 0x2e, 0x4e };
	int i;
	int rc = 0;
	static int port_internal = -1;

	if (port_internal > 0) {
		*port = (uint16_t)port_internal;
		return 1;
	}

	for (i = 0; i < ARRAY_SIZE(ports); i++) {
		uint8_t tmp8;

		/*
		 * Only after config mode has been successfully entered, the
		 * index port will read back the last value written to it.
		 * So we will attempt to enter config mode, set the index
		 * register, and see if the index register retains the value.
		 *
		 * Note: It seems to work "best" when using SIO_CHIPID1 as the
		 * index, and reading from the data port before reading the
		 * index port.
		 */
		mec1308_sio_enter(intf, ports[i]);
		io_write8(intf, ports[i], SIO_CHIPID1);
		io_read8(intf, ports[i] + 1, &tmp8);
		io_read8(intf, ports[i], &tmp8);
		if ((tmp8 != SIO_CHIPID1)) {
			in_sio_cfgmode = 0;
			continue;
		}

		port_internal = ports[i];
		break;
	}

	if (port_internal < 0) {
		lprintf(LOG_DEBUG, "%s: Port probing failed\n", __func__);
		rc = 0;
	} else {
		lprintf(LOG_DEBUG, "%s: Using port 0x%02x\n",
		                   __func__, port_internal);
		*port = (uint16_t)port_internal;
		rc = 1;
	}

	return rc;
}

/*
 * returns 1 to indicate mec1308 found
 * returns 0 otherwise
 */
int mec1308_detect(struct platform_intf *intf)
{
	uint16_t chipid = 0;
	uint16_t port;

	if (mec1308_get_sioport(intf, &port) <= 0)
		return 0;

	chipid = sio_read(intf, port, SIO_CHIPID1) << 8 |
	         sio_read(intf, port, SIO_CHIPID2);

	lprintf(LOG_INFO, "chipid: 0x%04x\n", chipid);
	switch(chipid){
	case 0x4d00:
	case 0x4d01:
		lprintf(LOG_DEBUG, "%s: found mec1308\n", __func__);
		break;
	case 0x0402:
		lprintf(LOG_DEBUG, "%s: found mec1310\n", __func__);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: failed to detect mec13xx\n", __func__);
		return 0;
	}

	return 1;
}

uint16_t mec1308_get_iobad(struct platform_intf *intf,
                           uint16_t port, uint8_t ldn)
{
	uint16_t iobad;
	uint8_t ldn_orig;
	unsigned int in_sio_cfgmode_before = in_sio_cfgmode;

	ldn_orig = sio_read(intf, port, SIO_LDNSEL);
	sio_write(intf, port, SIO_LDNSEL, ldn);

	iobad = sio_read(intf, port, 0x60) << 8 |
	        sio_read(intf, port, 0x61);

	sio_write(intf, port, SIO_LDNSEL, ldn_orig);

	if (!in_sio_cfgmode_before)
		mec1308_sio_exit(intf, port);
	return iobad;
}

const char *mec1308_sio_name(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;
	uint16_t ec_port;

	if (mec1308_get_sioport(intf, &ec_port) <= 0)
		return NULL;

	mec1308_sio_enter(intf, ec_port);
	id = get_sio_id(intf, ec_port);
	if (!id)
		return "Unknown";

	mec1308_sio_exit(intf, ec_port);
	return id->name;
}

const char *mec1308_sio_vendor(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;
	uint16_t ec_port;

	if (mec1308_get_sioport(intf, &ec_port) <= 0)
		return NULL;

	mec1308_sio_enter(intf, ec_port);
	id = get_sio_id(intf, ec_port);
	if (!id)
		return "Unknown";

	mec1308_sio_exit(intf, ec_port);
	return id->vendor;
}
