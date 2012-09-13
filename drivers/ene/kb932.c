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
#include <sys/time.h>

#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "drivers/superio.h"
#include "drivers/ene/kb932.h"

#define ENE_HWVER 0xa2
#define ENE_EDIID 0x02

static const int port_ene_bank   = 1;
static const int port_ene_offset = 2;
static const int port_ene_data   = 3;

static const uint16_t ene_hwver_addr = 0xff00;
static const uint16_t ene_ediid_addr = 0xff24;

/* KB932 uses i8042 OBF/IBF flag positions for all command/status interfaces */
static const uint8_t kb932_obf	= I8042_OBF;
static const uint8_t kb932_ibf	= I8042_IBF;

int kb932_wait_ibf_clear(struct platform_intf *intf)
{
	struct timeval begin, now;
	uint8_t ec_state;
	struct kb932_priv *ec_priv;

	MOSYS_DCHECK(intf->cb && intf->cb->ec && intf->cb->ec->priv);
	ec_priv = intf->cb->ec->priv;

	gettimeofday(&begin, NULL);
	do {
		io_read8(intf, ec_priv->csr, &ec_state);

		if (!(ec_state & kb932_ibf))
			return 0;

		gettimeofday(&now, NULL);
	} while (now.tv_sec - begin.tv_sec < ec_priv->cmd_timeout_ms * 1000);

	return -1;
}

int kb932_wait_obf_set(struct platform_intf *intf)
{
	struct timeval begin, now;
	uint8_t ec_state;
	struct kb932_priv *ec_priv;

	MOSYS_DCHECK(intf->cb && intf->cb->ec && intf->cb->ec->priv);
	ec_priv = intf->cb->ec->priv;

	gettimeofday(&begin, NULL);
	do {
		io_read8(intf, ec_priv->csr, &ec_state);

		if (ec_state & kb932_obf)
			return 0;

		gettimeofday(&now, NULL);
	} while (now.tv_sec - begin.tv_sec < ec_priv->cmd_timeout_ms * 1000);

	return -1;
}

/**
 * Read ene internal sram
 *
 * @param address       16bit sram address
 * @return              8bit sram data
 */
uint8_t ene_read(struct platform_intf *intf, uint16_t port,
			uint16_t address)
{
	uint8_t bank   = address >> 8;
	uint8_t offset = address & 0xff;
	uint8_t data = 0xff;

	io_write8(intf, port + port_ene_bank, bank);
	io_write8(intf, port + port_ene_offset, offset);
	io_read8(intf, port + port_ene_data, &data);
	return data;
}

/**
 * Write ene internal sram
 *
 * @param address       16bit sram address
 * @param data          8bit data
 */
void ene_write(struct platform_intf *intf, uint16_t port,
			uint16_t address, uint8_t data)
{
	uint8_t bank   = address >> 8;
	uint8_t offset = address & 0xff;

	io_write8(intf, port + port_ene_bank, bank);
	io_write8(intf, port + port_ene_offset, offset);
	io_write8(intf, port + port_ene_data, data);
}

const char *ene_name(enum ene_ec ec)
{
	const char *ret;

	switch (ec) {
	case ENE_KB932:
		ret = "KB932";
		break;
	case ENE_KB3940:
		ret = "KB3940";
		break;
	default:
		ret = "Unknown";
		break;
	}

	return ret;
}

/* returns ENE enum to indicate chip detected (0 if unknown or not detected) */
enum ene_ec ene_kb932_detect(struct platform_intf *intf, uint16_t port)
{
	uint8_t tmp8;
	static enum ene_ec ec;
	static int ec_probed = 0;

	if (ec_probed)
		return ec;

	tmp8 = ene_read(intf, port, ene_hwver_addr);
	lprintf(LOG_DEBUG, "%s: hwver: 0x%02x\n", __func__, tmp8);
	switch (tmp8) {
	case 0xa2:
		ec = ENE_KB932;
		break;
	case 0xa3:
		ec = ENE_KB3940;
		break;
	default:
		ec = ENE_UNKNOWN;
	}

	/* read EDIID if verbosity is high */
	if (mosys_get_verbosity() >= LOG_DEBUG) {
		tmp8 = ene_read(intf, port, ene_ediid_addr);
		lprintf(LOG_DEBUG, "%s: ediid: 0x%02x\n", __func__, tmp8);
	}

	ec_probed = 1;
	return ec;
}
