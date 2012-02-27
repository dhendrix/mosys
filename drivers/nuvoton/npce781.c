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

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "lib/math.h"

#include "drivers/superio.h"
#include "drivers/nuvoton/npce781.h"

/*
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
int npce781_get_sioport(struct platform_intf *intf, uint16_t *port)
{
	uint16_t ports[] = { 0x2e, 0x4e, 0x162e, 0x164e };
	int i;
	int rc = 0;
	static int port_internal = -1;

	if (port_internal > 0) {
		*port = (uint16_t)port_internal;
		return 1;
	}

	for (i = 0; i < ARRAY_SIZE(ports); i++) {
		if (sio_read(intf, ports[i], SIO_LDNSEL) != 0xff) {
			port_internal = ports[i];
			break;
		}
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
 * returns 1 to indicate npce781 found
 * returns 0 otherwise
 */
int npce781_detect(struct platform_intf *intf)
{
	uint16_t chipid = 0;
	uint16_t port;
	int ret = 0;

	if (npce781_get_sioport(intf, &port) <= 0)
		return 0;

	chipid = sio_read(intf, port, SIO_CHIPID1);
	lprintf(LOG_INFO, "chipid: 0x%04x\n", chipid);

	switch(chipid){
	case 0xfc:
		lprintf(LOG_DEBUG, "%s: found npce781\n", __func__);
		ret = 1;
		break;
	default:
		lprintf(LOG_DEBUG, "%s: failed to detect npce781\n", __func__);
		break;
	}

	return ret;
}

uint8_t npce781_read_csr(struct platform_intf *intf, uint16_t port,
                         uint8_t ldn, uint8_t reg)
{
	uint8_t ldn_orig, val;

	ldn_orig = sio_read(intf, port, SIO_LDNSEL);
	sio_write(intf, port, SIO_LDNSEL, ldn);

	val = sio_read(intf, port, reg);

	sio_write(intf, port, SIO_LDNSEL, ldn_orig);
	return val;
}
