/*
 * Copyright 2011, Google Inc.
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
 * mec1308_mbx.c: Shared EC mailbox interface code.
 */
 
#include <ctype.h>
#include <unistd.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/smsc/mec1308.h"
#include "drivers/superio.h"

#include "intf/io.h"

#define MEC1308_MBX_REG_CMD			0x82
#define MEC1308_MBX_REG_EXTCMD			0x83

#define MEC1308_MBX_CMD_FW_VERSION		0x83
#define MEC1308_MBX_CMD_FAN_RPM			0xBB

#define MEC1308_MBX_ALT_CMD_FW_VERSION		0x73
#define MEC1308_MBX_ALT_CMD_FAN_RPM		0x4B

#define MEC1308_MAX_TIMEOUT_US			2000000	/* arbitrarily picked */
#define MEC1308_DELAY_US			5000

#define MEC1308_MBX_CMD_PASSTHRU		0x55	/* start command */
#define MEC1308_MBX_CMD_PASSTHRU_SUCCESS	0xaa	/* success code */
#define MEC1308_MBX_CMD_PASSTHRU_FAIL		0xfe	/* failure code */
#define MEC1308_MBX_CMD_PASSTHRU_ENTER		"PathThruMode"	/* not a typo */
#define MEC1308_MBX_CMD_PASSTHRU_START		"Start"
#define MEC1308_MBX_CMD_PASSTHRU_EXIT		"End_Mode"

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

static int mbx_wait(struct platform_intf *intf, unsigned int us)
{
	int rc = 0;
	int us_elapsed = 0;
	uint8_t data;

	data = mbx_read(intf, MEC1308_MBX_REG_CMD);
	while (data) {
		if (us_elapsed >= MEC1308_MAX_TIMEOUT_US) {
			lprintf(LOG_ERR, "%s: EC timed out\n", __func__);
			return -1;
		}
		/* FIXME: This delay adds determinism to the delay period. It
		   was determined arbitrarily. */
		usleep(us);
		us_elapsed += us;
		data = mbx_read(intf, MEC1308_MBX_REG_CMD);
	}

	lprintf(LOG_SPEW, "%s: elapsed time: %d\n", __func__, us_elapsed);
	return rc;
}

static int mbx_write(struct platform_intf *intf, uint8_t idx, uint8_t data)
{
	int rc = 0;

	io_write8(intf, mbx_idx, idx);
	io_write8(intf, mbx_data, data);

	if (idx == MEC1308_MBX_REG_CMD)
		rc = mbx_wait(intf, MEC1308_DELAY_US);

	return rc;
}

static void mbx_clear(struct platform_intf *intf)
{
	int reg;

	for (reg = MEC1308_MBX_REG_DATA_START;
	     reg < MEC1308_MBX_REG_DATA_END;
	     reg++)
		mbx_write(intf, reg, 0x00);
	mbx_write(intf, MEC1308_MBX_REG_CMD, 0x00);
}

int mec1308_mbx_setup(struct platform_intf *intf)
{
	if (mec1308_get_sioport(intf, &ec_port) <= 0)
		return -1;

	mbx_idx = mec1308_get_iobad(intf, ec_port, MEC1308_LDN_MBX);
	mbx_data = mbx_idx + 1;

	lprintf(LOG_DEBUG, "%s: ec_port: 0x%04x, mbx_idx: 0x%04x, mbx_data: "
	                   "0x%04x\n", __func__, ec_port, mbx_idx, mbx_data);
	return 0;
}

void mec1308_mbx_teardown(struct platform_intf *intf)
{
	mec1308_sio_exit(intf, ec_port);
}

int mec1308_mbx_fw_version(struct platform_intf *intf, uint8_t *buf, int len)
{
	int i;
	uint8_t cmd = MEC1308_MBX_CMD_FW_VERSION;

	if (intf->cb->sys && intf->cb->sys->firmware_vendor) {
		const char *bios = intf->cb->sys->firmware_vendor(intf);
		if (bios && !strcasecmp(bios, "coreboot"))
			cmd = MEC1308_MBX_ALT_CMD_FW_VERSION;
		free((void *)bios);
	}

	memset(buf, 0, len);
	mbx_clear(intf);

	if (mbx_write(intf, MEC1308_MBX_REG_CMD, cmd) < 0)
		return -1;

	for (i = 0; i < len; i++) {
		uint8_t tmp = mbx_read(intf, MEC1308_MBX_REG_DATA_START + i);
		if (isalnum(tmp))
			buf[i] = tmp;
		else
			buf[i] = '\0';
	}

	return 0;
}

int mec1308_mbx_exit_passthru_mode(struct platform_intf *intf)
{
	int i;
	uint8_t tmp8;

	/* attempt to get out of passthru mode (assuming we're in it) */
	for (i = 0; i < strlen(MEC1308_MBX_CMD_PASSTHRU_EXIT); i++) {
		mbx_write(intf, MEC1308_MBX_REG_DATA_START + i,
		          MEC1308_MBX_CMD_PASSTHRU_EXIT[i]);
	}

	if (mbx_write(intf,
		      MEC1308_MBX_REG_CMD,
	              MEC1308_MBX_CMD_PASSTHRU)) {
		lprintf(LOG_DEBUG, "%s: exit passthru command timed out\n",
		        __func__);
		return -1;
	}

	tmp8 = mbx_read(intf, MEC1308_MBX_REG_DATA_START);
	lprintf(LOG_DEBUG, "%s: result: 0x%02x ", __func__, tmp8);
	if (tmp8 == MEC1308_MBX_CMD_PASSTHRU_SUCCESS) {
		lprintf(LOG_DEBUG, "(exited passthru mode)\n");
	} else if (tmp8 == MEC1308_MBX_CMD_PASSTHRU_FAIL) {
		lprintf(LOG_DEBUG, "(failed to exit passthru mode)\n");
	}

	return 0;
}
