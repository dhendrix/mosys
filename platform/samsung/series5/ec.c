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

#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>

#include "drivers/superio.h"
#include "drivers/smsc/mec1308.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#define ALEX_EC_DEFAULT_SIO_PORT	0x2e
#define ALEX_EC_DEFAULT_MBX_IOBAD	0xa00

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

#define ALEX_EC_CMD_PASSTHRU		0x55	/* force EC to process word */
#define ALEX_EC_CMD_PASSTHRU_SUCCESS	0xaa	/* success code for passthru */
#define ALEX_EC_CMD_PASSTHRU_FAIL	0xfe	/* failure code for passthru */
#define ALEX_EC_CMD_PASSTHRU_ENTER	"PathThruMode"	/* not a typo... */
#define ALEX_EC_CMD_PASSTHRU_START	"Start"
#define ALEX_EC_CMD_PASSTHRU_EXIT	"End_Mode"

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
 * samsung_series5_ec_testmbx - place mailbox in known state
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 to indicate failure
 */
static int alex_ec_exit_passthru_mode(struct platform_intf *intf)
{
	int i;
	uint8_t tmp8;

	/* attempt to get out of passthru mode (assuming we're in it) */
	for (i = 0; i < strlen(ALEX_EC_CMD_PASSTHRU_EXIT); i++) {
		mbx_write(intf, ALEX_EC_MBX_DATA_START + i,
		          ALEX_EC_CMD_PASSTHRU_EXIT[i]);
	}

	if (mbx_write(intf, ALEX_EC_MBX_CMD, ALEX_EC_CMD_PASSTHRU)) {
		lprintf(LOG_DEBUG, "%s: exit passthru command timed out\n",
		        __func__);
		return -1;
	}

	tmp8 = mbx_read(intf, ALEX_EC_MBX_DATA_START);
	lprintf(LOG_DEBUG, "%s: result: 0x%02x ", __func__, tmp8);
	if (tmp8 == ALEX_EC_CMD_PASSTHRU_SUCCESS) {
		lprintf(LOG_DEBUG, "(exited passthru mode)\n");
	} else if (tmp8 == ALEX_EC_CMD_PASSTHRU_FAIL) {
		lprintf(LOG_DEBUG, "(failed to exit passthru mode)\n");
	}

	return 0;
}

/*
 * samsung_series5_ec_name - return EC firmware name string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *samsung_series5_ec_name(struct platform_intf *intf)
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
 * samsung_series5_ec_vendor - return EC vendor string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *samsung_series5_ec_vendor(struct platform_intf *intf)
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
 * samsung_series5_ec_version - return allocated EC firmware version string
 *
 * @intf:	platform interface
 * @buf:	buffer to store version string in
 * @len:	bytes in version string
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static int samsung_series5_ec_fw_version(struct platform_intf *intf,
                                                uint8_t *buf, int len)
{
	int i;

	memset(buf, 0, len);

	mbx_clear(intf);
	if (mbx_write(intf, ALEX_EC_MBX_CMD, ALEX_EC_CMD_FW_VERSION) < 0)
		return -1;

	for (i = 0; i < len; i++)
		buf[i] = mbx_read(intf, ALEX_EC_MBX_DATA_START + i);
	return 0;
}

static const char *samsung_series5_ec_fw_version_wrapper(struct platform_intf *intf)
{
	static uint8_t version[ALEX_EC_MBX_DATA_LEN];
	int i, num_tries = 3;

	memset(version, 0, sizeof(version));

	for (i = 0; i < num_tries; i++) {
		int j, retry = 0;

		if (samsung_series5_ec_fw_version(intf, version,
		                                 ALEX_EC_MBX_DATA_LEN)) {
			lprintf(LOG_DEBUG, "%s: unable to issue command, "
			                   "attempting to unwedge EC\n",
			                   __func__);
			alex_ec_exit_passthru_mode(intf);
		}

		for (j = 0; j < ALEX_EC_MBX_DATA_LEN; j++) {
			if (!isascii(version[j])) {
				lprintf(LOG_DEBUG, "%s: bad output detected: \"%s\", "
				                   "attempting to unwedge EC\n",
						   __func__, version);
				alex_ec_exit_passthru_mode(intf);
			}
			retry = 1;
		}

		if (retry)
			continue;
		lprintf(LOG_DEBUG, "%s: ec firmware version: \"%s\"\n",
		                   __func__, version);
		break;
	}

	return version;
}

struct ec_cb samsung_series5_ec_cb = {
	.vendor		= samsung_series5_ec_vendor,
	.name		= samsung_series5_ec_name,
	.fw_version	= samsung_series5_ec_fw_version_wrapper,
};

int samsung_series5_ec_setup(struct platform_intf *intf)
{
	int rc = 0;

	/* invert logic -- mec1308_detect will return 1 if it finds an EC */
	if (!mec1308_detect(intf))
		rc = 1;

	if (mec1308_get_sioport(intf, &ec_port) <= 0) {
		lprintf(LOG_DEBUG, "%s: Could not probe EC via superio, assuming "
		                   "siocfg port 0x%02x and mailbox iobad "
				   "0x%04x\n", __func__,
		                   ALEX_EC_DEFAULT_SIO_PORT,
		                   ALEX_EC_DEFAULT_MBX_IOBAD);
		ec_port = ALEX_EC_DEFAULT_SIO_PORT;
		mbx_idx = ALEX_EC_DEFAULT_MBX_IOBAD;
		mbx_data = mbx_idx + 1;
	} else {
		mbx_idx = mec1308_get_iobad(intf, ec_port, MEC1308_LDN_MBX);
		mbx_data = mbx_idx + 1;

		lprintf(LOG_DEBUG, "%s: ec_port: 0x%04x, mbx_idx: 0x%04x, "
			           "mbx_data: 0x%04x\n", __func__, ec_port,
				   mbx_idx, mbx_data);
	}

	mec1308_sio_exit(intf, ec_port);

	/* Attempt to exit SPI passthru mode. This is a benign operation if
	   we are not already in passthru mode. */
	lprintf(LOG_DEBUG, "%s: attempting to exit passthru mode (this may"
	                   " fail safely)\n", __func__);
	alex_ec_exit_passthru_mode(intf);

	return rc;
}

void samsung_series5_ec_destroy(struct platform_intf *intf)
{
	/* FIXME: do we need this? */
	mec1308_sio_exit(intf, ec_port);
}
