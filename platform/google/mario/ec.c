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
#include <unistd.h>

#include "drivers/superio.h"
#include "drivers/ite/it8500.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

/* These are firmware-specific and not generically useful for it8500 */
#define MARIO_EC_MBX_CMD		0x02
#define MARIO_EC_MBX_IDX0		0x04
#define MARIO_EC_MBX_IDX1		0x05
#define MARIO_EC_MBX_DATA0		0x06
#define MARIO_EC_MBX_DATA1		0x07
#define MARIO_EC_MBX_OEM		0x31

#define MARIO_EC_CMD_FW_VERSION		0x10
#define MARIO_EC_CMD_READ_MBID		0x12
#define MARIO_EC_CMD_READ_ECRAM		0x22
#define MARIO_EC_CMD_WRITE_ECRAM	0x23

#define MARIO_EC_MAX_TIMEOUT_US		2000000	/* arbitrarily picked */
#define MARIO_EC_DELAY_US		1000

static void ec_wait(struct platform_intf *intf, uint16_t mbx_base)
{
	uint8_t tmp8;
	unsigned int t;

	for (t = 0; t < MARIO_EC_MAX_TIMEOUT_US; t += MARIO_EC_DELAY_US) {
		io_read8(intf, mbx_base + 1, &tmp8);
		if (tmp8 == 0x00)
			break;

		usleep(MARIO_EC_DELAY_US);
	}

	if (t >= MARIO_EC_MAX_TIMEOUT_US)
		lprintf(LOG_ERR, "%s: busy loop timed out\n", __func__);
	else
		lprintf(LOG_DEBUG, "%s: timeout value: %u\n", __func__, t);
}

static uint8_t ecram_read8(struct platform_intf *intf, uint16_t offset)
{
	uint16_t mbx_base;
	uint8_t tmp8 = 0;

	mbx_base = it8500_get_iobad(intf, IT8500_IOBAD1, IT8500_LDN_BRAM);
	io_write8(intf, mbx_base, MARIO_EC_MBX_CMD);
	ec_wait(intf, mbx_base);

	/* set EC RAM index pointers */
	io_write8(intf, mbx_base, MARIO_EC_MBX_IDX0);
	io_write8(intf, mbx_base + 1, offset & 0xff);
	io_write8(intf, mbx_base, MARIO_EC_MBX_IDX1);
	io_write8(intf, mbx_base + 1, (offset >> 8) & 0xff);

	/* issue "read EC RAM" command */
	io_write8(intf, mbx_base, MARIO_EC_MBX_CMD);
	io_write8(intf, mbx_base + 1, MARIO_EC_CMD_READ_MBID);
	ec_wait(intf, mbx_base);

	io_write8(intf, mbx_base, 0x06);	/* data port */
	io_read8(intf, mbx_base + 1, &tmp8);

	return tmp8;
}

/*
 * google_cr48_ec_name - return EC firmware name string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *google_cr48_ec_name(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;
	uint16_t port;

	if (it8500_get_sioport(intf, &port) <= 0)
		return "Unknown";

	id = get_sio_id(intf, port);
	if (!id)
		return "Unknown";

	return id->name;
}

/*
 * google_cr48_ec_vendor - return EC vendor string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *google_cr48_ec_vendor(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;
	uint16_t port;

	if (it8500_get_sioport(intf, &port) <= 0)
		return "Unknown";

	id = get_sio_id(intf, port);
	if (!id)
		return "Unknown";

	return id->vendor;
}

int google_cr48_ec_setup(struct platform_intf *intf)
{
	int rc = 0;

	/* invert logic -- it8500_detect will return 1 if it finds an it8500 EC */
	if (!it8500_detect(intf))
		rc = 1;

	return rc;
}

/*
 * google_cr48_ec_mbid - return allocated mainboard ID string
 *
 * @intf:	platform interface
 *
 * Mainboard ID string is determined based upon EC strapping.
 *
 * returns 0 if successful
 * returns <0 if failure
 */
char *google_cr48_ec_mbid(struct platform_intf *intf)
{
	uint8_t tmp8;
	char *ret = NULL;

	tmp8 = ecram_read8(intf, 0x051e);
	switch(tmp8 & 0x07) {
	default:
		ret = mosys_strdup("Unknown");
		break;
	}

	lprintf(LOG_DEBUG, "mbid 0x%02x: %s\n", tmp8 & 0x0f, ret);
	return ret;
}

/*
 * google_cr48_ec_version - return allocated EC firmware version
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *google_cr48_ec_fw_version(struct platform_intf *intf)
{
	uint16_t mbx_base;
	static char version[5];

	mbx_base = it8500_get_iobad(intf, IT8500_IOBAD1, IT8500_LDN_BRAM);
	io_write8(intf, mbx_base, MARIO_EC_MBX_CMD);
	io_write8(intf, mbx_base + 1, MARIO_EC_CMD_FW_VERSION);
	ec_wait(intf, mbx_base);

	io_write8(intf, mbx_base, MARIO_EC_MBX_IDX0);
	io_read8(intf, mbx_base + 1, &version[0]);

	io_write8(intf, mbx_base, MARIO_EC_MBX_IDX1);
	io_read8(intf, mbx_base + 1, &version[1]);

	io_write8(intf, mbx_base, MARIO_EC_MBX_DATA0);
	io_read8(intf, mbx_base + 1, &version[2]);

	io_write8(intf, mbx_base, MARIO_EC_MBX_DATA1);
	io_read8(intf, mbx_base + 1, &version[3]);

	version[4] = '\0';
	return version;
}

struct legacy_ec_cb google_cr48_ec_cb = {
	.vendor		= google_cr48_ec_vendor,
	.name		= google_cr48_ec_name,
	.fw_version	= google_cr48_ec_fw_version,
};
