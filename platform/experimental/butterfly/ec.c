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

#include "drivers/ene/kb932.h"
#include "intf/io.h"
#include "lib/math.h"

#include "butterfly.h"

/*
 * Butterfly EC firmware specific constants
 */
#define	BUTTERFLY_EC_CMD_TIMEOUT	500
#define BUTTERFLY_EC_REG_BASE		0x380

#define BUTTERFLY_ECRAM_CPU_TMP		0x78
#define BUTTERFLY_ECRAM_GPU_TMP		0x79
#define BUTTERFLY_ECRAM_FANTACH_LO	0x7c
#define BUTTERFLY_ECRAM_FANTACH_HI	0x7d
#define BUTTERFLY_ECRAM_FW_VERSION	0xba
#define BUTTERFLY_ECRAM_FW_VERSION_LEN	6

static enum ene_ec butterfly_ec;

static uint8_t read_ecram(struct platform_intf *intf,
			uint8_t offset, uint8_t *data)
{
	struct kb932_priv *ec_priv;

	MOSYS_DCHECK(intf->cb && intf->cb->ec && intf->cb->ec->priv);
	ec_priv = intf->cb->ec->priv;

	if (kb932_wait_ibf_clear(intf))
		return -1;

	if (io_write8(intf, ec_priv->csr, KB932_CMD_READ_ECRAM))
		return -1;

	if (kb932_wait_ibf_clear(intf))
		return -1;

	if (io_write8(intf, ec_priv->data, offset) < 0)
		return -1;

	if (kb932_wait_obf_set(intf))
		return -1;

	if (io_read8(intf, ec_priv->data, data))
		return -1;

	return 0;
}

static const char *butterfly_ec_vendor(struct platform_intf *intf)
{
	return "ENE";
}

static const char *butterfly_ec_name(struct platform_intf *intf)
{
	return ene_name(butterfly_ec);
}

/**
 * Get butterfly vendor specific fw version string (ASCII)
 */
static const char *butterfly_ec_fw_version(struct platform_intf *intf)
{
	int i;
	static char version[BUTTERFLY_ECRAM_FW_VERSION_LEN + 1];

	for (i = 0; i < ARRAY_SIZE(version); i++)
		read_ecram(intf, BUTTERFLY_ECRAM_FW_VERSION + i, &version[i]);
	version[i] = '\0';

	return (const char *)version;
}

int butterfly_ec_setup(struct platform_intf *intf)
{
	butterfly_ec = ene_kb932_detect(intf, BUTTERFLY_EC_REG_BASE);
	if (butterfly_ec == ENE_UNKNOWN)
		return -1;

	lprintf(LOG_DEBUG, "%s: Found KB3940\n", __func__);
	return 0;
}

struct kb932_priv butterfly_ec_priv = {
	.csr		= ACPI_CSR,
	.data		= ACPI_DATA,
	.reg_base	= BUTTERFLY_EC_REG_BASE,
	.cmd_timeout_ms	= BUTTERFLY_EC_CMD_TIMEOUT,
};

struct ec_cb butterfly_ec_cb = {
	.vendor		= &butterfly_ec_vendor,
	.name		= &butterfly_ec_name,
	.fw_version	= &butterfly_ec_fw_version,
	.priv		= &butterfly_ec_priv,
};
