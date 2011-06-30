/*
 * Copyright (C) 2011 Google Inc.
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
#include "drivers/nuvoton/npce781.h"

#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

static uint16_t ec_port;
static uint16_t mbx_csr;
static uint16_t mbx_data;

enum buffer_state {
	CLEAR	= 0,
	SET	= 1
};

static int mbx_wait(struct platform_intf *intf,
                    uint8_t mask, enum buffer_state state)
{
	int timeout = 250000;	/* FIXME: get better timeout info */
	int step = 1000;

	while (timeout) {
		uint8_t tmp8;

		if (io_read8(intf, mbx_csr, &tmp8) < 0)
			return -1;
		if ((tmp8 & mask) == state)
			break;

		timeout -= step;
		usleep(step);
	}

	if (!timeout)
		return -1;

	return 0;
}

/*
 * acer_chromia700_ec_name - return EC name
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *acer_chromia700_ec_name(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;

	id = get_sio_id(intf, ec_port);
	if (!id)
		return "Unknown";

	return id->name;
}

/*
 * acer_chromia700_ec_vendor - return EC vendor string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *acer_chromia700_ec_vendor(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;

	lprintf(LOG_DEBUG, "%s: using port %04x\n", __func__, ec_port);

	id = get_sio_id(intf, ec_port);
	if (!id)
		return "Unknown";

	return id->vendor;
}

/*
 * acer_chromia700_ec_version - return allocated EC firmware version string
 *
 * @intf:	platform interface
 * @buf:	buffer to store version string in
 * @len:	bytes in version string
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *acer_chromia700_ec_fw_version(struct platform_intf *intf)
{
	static char version[5];	/* "X.YZ" */

	/*
	 * EC F/W revision need access from EC Mailbox I/O PORT 6C/68H and
	 * behavior sane as KBC Interface. application need wait IBF clear
	 * before send new command or data to I/O 6CH or 68H, and wait OBF
	 * assert to read data.
	 *
	 * The EC revision format is ASCII KEY "X.YZ"
	 * 1) wait IBF clear , PORT 6CH <- 0xFA , wait IBF clear , PORT 68H <- 0x05
	 * 2) wait IBF clear , PORT 6CH <- 0xFB . wait OBF assert , read PORT 68H , get "X"
	 * 3) wait IBF clear , PORT 6CH <- 0xFA , wait IBF clear , PORT 68H <- 0x07
	 * 4) wait IBF clear , PORT 6CH <- 0xFB . wait OBF assert , read PORT 68H , get "Y"
	 * 5) wait IBF clear , PORT 6CH <- 0xFA , wait IBF clear , PORT 68H <- 0x08
	 * 6) wait IBF clear , PORT 6CH <- 0xFB . wait OBF assert , read PORT 68H , get "Z"
	 */

	/* step 1 */
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_csr, 0xfa);
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_data, 0x05);
	/* step 2 */
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_csr, 0xfb);
	if (mbx_wait(intf, NPCE781_MBX_CSR_OBF, SET)) return "unknown";
	io_read8(intf, mbx_data, &version[0]);

	version[1] = '.';

	/* step 3 */
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_csr, 0xfa);
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_data, 0x07);
	/* step 4 */
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_csr, 0xfb);
	if (mbx_wait(intf, NPCE781_MBX_CSR_OBF, SET)) return "unknown";
	io_read8(intf, mbx_data, &version[2]);

	/* step 5 */
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_csr, 0xfa);
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_data, 0x08);
	/* step 6 */
	if (mbx_wait(intf, NPCE781_MBX_CSR_IBF, CLEAR)) return "unknown";
	io_write8(intf, mbx_csr, 0xfb);
	if (mbx_wait(intf, NPCE781_MBX_CSR_OBF, SET)) return "unknown";
	io_read8(intf, mbx_data, &version[3]);

	version[4] = '\0';

	return version;
}

struct ec_cb acer_chromia700_ec_cb = {
	.vendor		= acer_chromia700_ec_vendor,
	.name		= acer_chromia700_ec_name,
	.fw_version	= acer_chromia700_ec_fw_version,
};

int acer_chromia700_ec_setup(struct platform_intf *intf)
{
	if (npce781_detect(intf) != 1)
		return -1;

	if (npce781_get_sioport(intf, &ec_port) <= 0)
		return -1;

	/* For this platform, we use the mailbox registers in PM2. Other
	 * platforms with the NPCE781 may use different set of mailbox
	 * registers */
	mbx_data = (npce781_read_csr(intf, ec_port, NPCE781_LDN_PM2, 0x60) << 8) |
	            npce781_read_csr(intf, ec_port, NPCE781_LDN_PM2, 0x61);

	mbx_csr = (npce781_read_csr(intf, ec_port, NPCE781_LDN_PM2, 0x62) << 8) |
	           npce781_read_csr(intf, ec_port, NPCE781_LDN_PM2, 0x63);

	lprintf(LOG_DEBUG, "%s: ec_port: 0x%04x, mbx_data: 0x%04x, "
		           "mbx_csr: 0x%04x\n", __func__, ec_port,
			   mbx_data, mbx_csr);

	return 0;
}

void acer_chromia700_ec_destroy(struct platform_intf *intf)
{
	/* FIXME: do we need this? */
}
