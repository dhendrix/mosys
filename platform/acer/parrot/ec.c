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

#include <unistd.h>
#include <sys/time.h>

#include "mosys/log.h"
#include "mosys/platform.h"
#include "drivers/superio.h"
#include "drivers/ene/kb932.h"
#include "intf/io.h"

#include "parrot.h"


/**
 * Parrot EC firmware specific constants
 */
#define PARROT_EC_CMD		0x6c
#define PARROT_EC_DATA		0x68
#define PARROT_ECRAM_PORT	0xfd60

#define EC_CMD_TIMEOUT_MS	4000
#define EC_CMD_FW_VERSION	0x51

/**
 * Wait for EC firmware input buffer empty
 *
 * @param intf          platform_intf
 * @return 0            success
 * @return -1           timeout
 */
static int ec_wait_input(struct platform_intf *intf)
{
	if (kb932_wait_ibf_clear(intf) != 1)
		return -1;
	return 0;
}

/**
 * Wait for EC firmware output buffer empty
 *
 * @param intf          platform_intf
 * @return 0            success
 * @return -1           timeout
 */
static int ec_wait_output(struct platform_intf *intf)
{
	if (kb932_wait_obf_set(intf) != 1)
		return -1;
	return 0;
}

/* Write command to ec firmware command port */
static int ec_cmd(struct platform_intf *intf, uint8_t cmd)
{
	if (ec_wait_input(intf))
		return -1;

	io_write8(intf, PARROT_EC_CMD, cmd);
	return 0;
}

/* Read data from ec firmware data port */
static int ec_read(struct platform_intf *intf, uint16_t port, uint8_t *data)
{
	if (ec_wait_output(intf))
		return -1;

	io_read8(intf, port, data);
	return 0;
}


static const char *parrot_ec_vendor(struct platform_intf *intf)
{
	return ene_kb932_detect(intf, PARROT_ECRAM_PORT) ?
		"ENE" : "Unknown";
}

static const char *parrot_ec_name(struct platform_intf *intf)
{
	return ene_kb932_detect(intf, PARROT_ECRAM_PORT) ?
		"KB932" : "Unknown";
}

static void bcd_to_ascii(uint8_t bcd, char *ascii)
{
	uint8_t digit;

	/* high nibble first */
	digit = bcd >> 4;
	if (digit <= 9)
		ascii[0] = digit + '0';

	digit = bcd & 0xf;
	if (digit <= 9)
		ascii[1] = digit + '0';
}

/**
 * Get parrot vendor specific fw version string
 *
 * Parrot ec firmware version format: '00BEmnnArr'
 *   '00BE' - hardware type, parrot
 *   'mnnA' - m : major, 0 ~ 9
 *            nn: minor, 00~99 binary coded dicimal
 *   'rr'   - rev, 00~99 binary coded dicimal
 */
static const char *parrot_ec_fw_version(struct platform_intf *intf)
{
	uint8_t major, minor, rev;
	static char version[11];

	if (ene_kb932_detect(intf, PARROT_ECRAM_PORT))
		memcpy(version, "00BExxxAxx", 11);
	else
		return "Unknown";

	/* get 3 bytes firmware version */
	ec_cmd(intf, EC_CMD_FW_VERSION);

	/* major range: 0 ~ 9 */
	if (!ec_read(intf, PARROT_EC_DATA, &major)) {
		if (major < 10)
			version[4] = major + '0';
	}
	/* minor range: 00 ~ 99 */
	if (!ec_read(intf, PARROT_EC_DATA, &minor))
		bcd_to_ascii(minor, version + 5);
	/* rev range: 00 ~ 99 */
	if (!ec_read(intf, PARROT_EC_DATA, &rev))
		bcd_to_ascii(rev, version + 8);

	return version;
}

struct kb932_priv parrot_ec_priv = {
	.csr		= PARROT_EC_CMD,
	.data		= PARROT_EC_DATA,
	.reg_base	= PARROT_ECRAM_PORT,
	.cmd_timeout_ms	= EC_CMD_TIMEOUT_MS,
};

struct ec_cb parrot_ec_cb = {
	.vendor		= parrot_ec_vendor,
	.name		= parrot_ec_name,
	.fw_version	= parrot_ec_fw_version,
	.priv		= &parrot_ec_priv,
};

