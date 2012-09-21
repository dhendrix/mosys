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

#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/superio.h"
#include "drivers/ite/it8500.h"
#include "intf/io.h"
#include "lib/acpi.h"

/* These are firmware-specific and not generically useful for it8500 */
#define STOUT_ECMEM_FW_VERSION_MSB	0xe8
#define STOUT_ECMEM_FW_VERSION_LSB	0xe9

#define STOUT_EC_CMD_TIMEOUT_MS		1000

/* for i8042-style wait commands */
struct i8042_host_intf stout_acpi_intf = {
	.csr	= ACPI_EC_SC,
	.data	= ACPI_EC_DATA,
};

int stout_wait_ibf_clear(struct platform_intf *intf)
{
	return i8042_wait_ibf_clear(intf, &stout_acpi_intf,
			STOUT_EC_CMD_TIMEOUT_MS);
}

int stout_wait_obf_set(struct platform_intf *intf)
{
	return i8042_wait_obf_set(intf, &stout_acpi_intf,
			STOUT_EC_CMD_TIMEOUT_MS);
}

/* returns 0 to indicate success, <0 to indicate failure */
static int ecram_read(struct platform_intf *intf,
			uint8_t offset, uint8_t *data)
{
	if (stout_wait_ibf_clear(intf) != 1)
		return -1;
	if (io_write8(intf, ACPI_EC_SC, ACPI_RD_EC) < 0)
		return -1;
	if (stout_wait_ibf_clear(intf) != 1)
		return -1;
	if (io_write8(intf, ACPI_EC_DATA, offset) < 0)
		return -1;
	if (stout_wait_obf_set(intf) != 1)
		return -1;
	if (io_read8(intf, ACPI_EC_DATA, data) < 0)
		return -1;

	return 0;
}

/*
 * stout_ec_version - return allocated EC firmware version
 *
 * @intf:	platform interface
 *
 * returns pointer to version string if successful
 * returns NULL to indicate failure
 */
static const char *stout_ec_fw_version(struct platform_intf *intf)
{
	int major, minor;
	char revision;
	static char version[7];
	uint8_t msb, lsb;

	if (ecram_read(intf, STOUT_ECMEM_FW_VERSION_MSB, &msb) < 0)
		return NULL;
	if (ecram_read(intf, STOUT_ECMEM_FW_VERSION_LSB, &lsb) < 0)
		return NULL;

	major = (msb >> 4) & 0xf;
	minor = ((msb & 0xf) << 4) | ((lsb >> 4) & 0xf);
	revision = 'A' + (lsb & 0xf);

	memset(version, 0, sizeof(version));
	sprintf(version, "%x.%02x%c", major, minor, revision);
	return version;
}

/*
 * stout_ec_name - return EC firmware name string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *stout_ec_name(struct platform_intf *intf)
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
 * stout_ec_vendor - return EC vendor string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *stout_ec_vendor(struct platform_intf *intf)
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

int stout_ec_setup(struct platform_intf *intf)
{
	int rc = 0;

	/* invert logic -- it8500_detect will return 1 if it finds an it8500 EC */
	if (!it8500_detect(intf))
		rc = 1;

	return rc;
}

struct ec_cb stout_ec_cb = {
	.vendor		= stout_ec_vendor,
	.name		= stout_ec_name,
	.fw_version	= stout_ec_fw_version,
};
