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

#include "stout.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/superio.h"
#include "drivers/ite/it8500.h"
#include "intf/io.h"
#include "lib/acpi.h"

#define STOUT_EC_CMD_TIMEOUT_MS		3000

static struct i8042_host_intf stout_acpi_intf_rw = {
	/* use sideband ports to avoid racing with kernel ACPI driver */
	.csr	= 0x6c,
	.data	= 0x68,
};

static struct i8042_host_intf stout_acpi_intf_ro = {
	/* try regular ports in case EC is in RO */
	.csr	= ACPI_EC_SC,
	.data	= ACPI_EC_DATA,
};

/* for i8042-style wait commands */
static struct i8042_host_intf *stout_acpi_intf = &stout_acpi_intf_rw;

static int stout_wait_ibf_clear(struct platform_intf *intf)
{
	return i8042_wait_ibf_clear(intf, stout_acpi_intf,
			STOUT_EC_CMD_TIMEOUT_MS);
}

static int stout_wait_obf_set(struct platform_intf *intf)
{
	return i8042_wait_obf_set(intf, stout_acpi_intf,
			STOUT_EC_CMD_TIMEOUT_MS);
}

/*
 * ec_command - sends command to EC, with optional read/write parameters.
 *
 * @inf:		platform interface
 * command:		EC command
 * @input_data: 	optional input data to send (NULL if none)
 * input_len:		length of @input_data (0 if input_data == NULL)
 * @output_data:	optional output data to send (NULL if none)
 * output_len:		length of @output_data (0 if output_data == NULL)
 *
 * Returns 0 to indicate success, <0 to indicate failure.
 */
int ec_command(struct platform_intf *intf, stout_ec_command command,
			uint8_t *input_data, uint8_t input_len,
			uint8_t *output_data, uint8_t output_len )
{
	int i;

	if (stout_wait_ibf_clear(intf) != 1)
		return -1;
	if (io_write8(intf, stout_acpi_intf->csr, command) < 0)
		return -1;

	if (input_data != NULL) {
		for (i = 0; i < input_len; i++) {
			if (stout_wait_ibf_clear(intf) != 1)
				return -1;
			if (io_write8(intf, stout_acpi_intf->data,
					input_data[i] ) < 0)
				return -1;
		}
	}

	if (output_data != NULL) {
		for (i = 0; i < output_len; i++) {
			if (stout_wait_obf_set(intf) != 1)
				return -1;
			if (io_read8(intf, stout_acpi_intf->data,
					&output_data[i] ) < 0)
				return -1;
		}
	}

	return 0;
}


/* returns 0 to indicate success, <0 to indicate failure */
int ecram_read(struct platform_intf *intf,
			stout_ec_mem_addr address, uint8_t *data,
			stout_ec_command cmd)
{
	return ec_command(intf, cmd, (int8_t *)&address, 1,
				data, 1);
}

/* returns 0 to indicate success, <0 to indicate failure */
int ecram_write(struct platform_intf *intf,
			stout_ec_mem_addr address, uint8_t data)
{
	uint8_t write_data[2] = {address, data};

	return ec_command(intf, STOUT_ECCMD_MEM_WRITE, write_data, 2,
				NULL, 0);
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
	stout_ec_command cmd = STOUT_ECCMD_MEM_READ;

	if (ecram_read(intf, STOUT_ECMEM_FW_VERSION_MSB, &msb, cmd) < 0) {
		/* Command failed, but we might be in RO. */
		stout_acpi_intf = &stout_acpi_intf_ro;
		cmd = ACPI_RD_EC;
		lprintf(LOG_DEBUG, "%s: Retry w/ RO ports\n", __func__);

		if (ecram_read(intf, STOUT_ECMEM_FW_VERSION_MSB, &msb, cmd) < 0)
			return NULL;
	}
	if (ecram_read(intf, STOUT_ECMEM_FW_VERSION_LSB, &lsb, cmd) < 0)
		return NULL;

	major = (msb >> 4) & 0xf;
	minor = ((msb & 0xf) << 4) | ((lsb >> 4) & 0xf);

	/* Revision: Zero indicates no revision character */
	if ((lsb & 0xf) == 0)
		revision = '\0';
	else
		revision = 'A' + (lsb & 0xf) - 1;

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

	/* invert logic: it8500_detect will return 1 if it finds an it8500 EC */
	if (!it8500_detect(intf))
		rc = 1;

	return rc;
}

struct ec_cb stout_ec_cb = {
	.vendor		= stout_ec_vendor,
	.name		= stout_ec_name,
	.fw_version	= stout_ec_fw_version,
};
