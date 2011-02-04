/*
 * Copyright (C) 2010 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Google Inc. or the names of contributors or
 * licensors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * GOOGLE INC AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Note: This file shares some code with the Flashrom project.
 */

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "lib/math.h"

#include "drivers/superio.h"
#include "drivers/smsc/mec1308.h"

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

		mec1308_sio_enter(intf, ports[i]);

		/*
		 * If entry is successful, the data port will read back 0x00
		 * and the index port will read back the last value written to
		 * it (the key itself).
		 */
		io_read8(intf, ports[i], &tmp8);
		if (tmp8 != MEC1308_SIO_ENTRY_KEY) {
			in_sio_cfgmode = 0;
			continue;
		}

		io_read8(intf, ports[i] + 1, &tmp8);
		if (tmp8 != 0x00) {
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
	default:
		lprintf(LOG_DEBUG, "%s: failed to detect mec1308\n", __func__);
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
