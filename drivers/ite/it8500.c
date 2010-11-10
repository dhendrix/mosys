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
 */

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "lib/math.h"

#include "drivers/superio.h"
#include "drivers/ite/it8500.h"

/*
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
int it8500_get_sioport(struct platform_intf *intf, uint16_t *port)
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
 * returns 1 to indicate it8500 found
 * returns 0 otherwise
 */
int it8500_detect(struct platform_intf *intf)
{
	uint8_t ports[] = { 0x2e, 0x4e };
	int i;
	uint16_t chipid = 0;

	for (i = 0; i < ARRAY_SIZE(ports); i++) {
		if (sio_read(intf, ports[i], SIO_LDNSEL) != 0xff) {
			port = ports[i];
			break;
		}
	}

	if (port < 0) {
		lprintf(LOG_DEBUG, "%s: Port probing failed\n", __func__);
		return -1;
	} else {
		lprintf(LOG_DEBUG, "%s: Using port %d\n", __func__, port);
	}

	chipid = sio_read(intf, port, SIO_CHIPID1) << 8 |
	         sio_read(intf, port, SIO_CHIPID2);

	lprintf(LOG_INFO, "chipid: 0x%04x\n", chipid);
	switch(chipid){
	case 0x8500:
		lprintf(LOG_DEBUG, "%s: found it8500\n", __func__);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: failed to detect it8500\n", __func__);
		return 0;
	}

	return 1;
}

uint16_t it8500_get_iobad(struct platform_intf *intf, int bank, uint8_t ldn)
{
	uint8_t iobad_msb, iobad_lsb;
	uint16_t iobad;
	uint8_t ldn_orig;

	ldn_orig = sio_read(intf, port, SIO_LDNSEL);
	sio_write(intf, port, SIO_LDNSEL, ldn);

	switch(bank) {
	case IT8500_IOBAD0:
		iobad_msb = sio_read(intf, port, 0x60);
		iobad_lsb = sio_read(intf, port, 0x61);
		break;
	case IT8500_IOBAD1:
		iobad_msb = sio_read(intf, port, 0x62);
		iobad_lsb = sio_read(intf, port, 0x63);
		break;
	default:
		lprintf(LOG_ERR, "%s: invalid io address bank: %d\n",
		                 __func__, bank);
		goto it8500_get_iobad_exit;
	}


	iobad = (iobad_msb << 8) | iobad_lsb;
	lprintf(LOG_DEBUG, "%s: io base address for ldn 0x%02x: 0x%04x\n",
	                   __func__, ldn, iobad);

it8500_get_iobad_exit:
	sio_write(intf, port, SIO_LDNSEL, ldn_orig);
	return iobad;
}
