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
#include <unistd.h>

#include "drivers/superio.h"
#include "drivers/smsc/mec1308.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

/* These are firmware-specific and not generically useful for mec1308 */
#define ALEX_EC_MBX_CMD			0x82
#define ALEX_EC_MBX_EXTCMD		0x83
#define ALEX_EC_MBX_DATA_START		0x84
#define ALEX_EC_MBX_DATA_END		0x91
#define ALEX_EC_MBX_DATA_LEN		(ALEX_EC_MBX_DATA_END - \
                                         ALEX_EC_MBX_DATA_START)

#define ALEX_EC_CMD_FW_VERSION		0x83
#define ALEX_EC_CMD_FAN_RPM		0xBB

#define ALEX_EC_MAX_TIMEOUT_US		2000000	/* arbitrarily picked */
#define ALEX_EC_DELAY_US		5000

static uint16_t ec_port;
static uint16_t mbx_idx;
static uint16_t mbx_data;

static uint8_t mbx_read(struct platform_intf *intf, uint8_t idx)
{
	uint8_t data;

	io_write8(intf, mbx_idx, idx);
	io_read8(intf, mbx_data, &data);

	return data;
}

static int mbx_wait(struct platform_intf *intf)
{
	int rc = 0;
	int us_elapsed = 0;
	uint8_t data;

	data = mbx_read(intf, ALEX_EC_MBX_CMD);
	while (data) {
		if (us_elapsed >= ALEX_EC_MAX_TIMEOUT_US) {
			lprintf(LOG_ERR, "%s: EC timed out\n", __func__);
			return -1;
		}
		/* FIXME: This delay adds determinism to the delay period. It
		   was determined arbitrarily. */
		usleep(ALEX_EC_DELAY_US);
		us_elapsed += ALEX_EC_DELAY_US;
		data = mbx_read(intf, ALEX_EC_MBX_CMD);
	}

	lprintf(LOG_DEBUG, "%s: elapsed time: %d\n", __func__, us_elapsed);
	return rc;
}

static int mbx_write(struct platform_intf *intf, uint8_t idx, uint8_t data)
{
	int rc = 0;

	io_write8(intf, mbx_idx, idx);
	io_write8(intf, mbx_data, data);

	if (idx == ALEX_EC_MBX_CMD)
		rc = mbx_wait(intf);

	return rc;
}

static void mbx_clear(struct platform_intf *intf)
{
	int reg;

	for (reg = ALEX_EC_MBX_DATA_START; reg < ALEX_EC_MBX_DATA_END; reg++)
		mbx_write(intf, reg, 0x00);
	mbx_write(intf, ALEX_EC_MBX_CMD, 0x00);
}

/*
 * alex_pinetrail_ec_name - return EC firmware name string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *alex_pinetrail_ec_name(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;

	mec1308_sio_enter(intf, ec_port);
	id = get_sio_id(intf, ec_port);
	if (!id)
		return "Unknown";

	mec1308_sio_exit(intf, ec_port);
	return id->name;
}

/*
 * alex_pinetrail_ec_vendor - return EC vendor string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *alex_pinetrail_ec_vendor(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;

	mec1308_sio_enter(intf, ec_port);
	lprintf(LOG_DEBUG, "%s: using port %04x\n", __func__, ec_port);

	id = get_sio_id(intf, ec_port);
	if (!id)
		return "Unknown";

	mec1308_sio_exit(intf, ec_port);
	return id->vendor;
}

/*
 * alex_pinetrail_ec_version - return allocated EC firmware version string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *alex_pinetrail_ec_fw_version(struct platform_intf *intf)
{
	int i;
	static uint8_t version[ALEX_EC_MBX_DATA_LEN];

	mbx_clear(intf);
	mbx_write(intf, ALEX_EC_MBX_CMD, ALEX_EC_CMD_FW_VERSION);

	/* Firmware version is pre-determined to be 10 bytes */
	memset(version, 0, sizeof(version));
	for (i = 0; i < ALEX_EC_MBX_DATA_LEN; i++)
		version[i] = mbx_read(intf, ALEX_EC_MBX_DATA_START + i);
	return version;
}

struct ec_cb alex_pinetrail_ec_cb = {
	.vendor		= alex_pinetrail_ec_vendor,
	.name		= alex_pinetrail_ec_name,
	.fw_version	= alex_pinetrail_ec_fw_version,
};

int alex_pinetrail_ec_setup(struct platform_intf *intf)
{
	int rc = 0;

	/* invert logic -- mec1308_detect will return 1 if it finds an EC */
	if (!mec1308_detect(intf))
		rc = 1;

	if (mec1308_get_sioport(intf, &ec_port) <= 0)
		return -1;

	mbx_idx = mec1308_get_iobad(intf, ec_port, MEC1308_LDN_MBX);
	mbx_data = mbx_idx + 1;

	lprintf(LOG_DEBUG, "%s: ec_port: 0x%04x, mbx_idx: 0x%04x, "
		           "mbx_data: 0x%04x\n", __func__, ec_port,
			   mbx_idx, mbx_data);
	mec1308_sio_exit(intf, ec_port);
	return rc;
}

void alex_pinetrail_ec_destroy(struct platform_intf *intf)
{
	/* FIXME: do we need this? */
	mec1308_sio_exit(intf, ec_port);
}
