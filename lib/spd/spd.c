/* Copyright 2012, Google Inc.
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
 * DIMM Serial Presence Detect interface normal access is via I2C but some
 * platforms hide SPD behind memory controller and other access method must be
 * used.
 */

#define _LARGEFILE64_SOURCE
#include <arpa/inet.h>	/* ntohl() */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "mosys/alloc.h"
#include "mosys/list.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/i2c.h"
#include "lib/cbfs_core.h"
#include "lib/smbios.h"
#include "lib/smbios_tables.h"
#include "lib/spd.h"

static spd_raw_override spd_raw_access_override;

const char *ddr_freq_prettyprint[] = {
	[DDR_FREQ_UNKNOWN] = "Unknown",
	[DDR_333] = "667",
	[DDR_400] = "800",
	[DDR_533] = "1066",
	[DDR_667] = "1333",
	[DDR_800] = "1600",
	[DDR_933] = "1866",
	[DDR_1067] = "2133",
};

/*
 * spd_raw_i2c  -  Read/write to/from SPD via I2C
 *
 * @intf:	platform interface
 * @bus:        bus to read from
 * @address:    address on bus
 * @reg:	configuration register
 * @length:	number of bytes to read
 * @data:       data buffer
 * @rw:         1=write, 0=read
 *
 * returns number of bytes written or read
 * returns <0 to indicate error
 */
static int spd_raw_i2c(struct platform_intf *intf, int bus,
		       int address, int reg, int length, void *data, int rw)
{
	if (!intf->op->i2c) {
		return -ENOSYS;
	}

	switch (rw) {
	case SPD_READ:
		return intf->op->i2c->smbus_read_reg(intf, bus, address,
					             reg, length, data);
	case SPD_WRITE:
		return intf->op->i2c->smbus_write_reg(intf, bus, address,
						      reg, length, data);
	}

	return -1;
}

/* spd_raw_access - read/write access method to SPDs
 *
 * @intf:  platform interface
 * @bus:  SMBus number of requested SPD
 * @address: SMBus address of requested SPD
 * @reg:  register in SPD to perform access on
 * @length:  length of access to perform
 * @data: pointer to buffer that is either filled or read from to do the access
 * @rw:  specify SPD operation (SPD_READ or SPD_WRITE)
 *
 * returns 0 on success, < 0 on error.
 */
int spd_raw_access(struct platform_intf *intf, int bus, int address,
                   int reg, int length, void *data, int rw)
{
	if (spd_raw_access_override != NULL) {
		return spd_raw_access_override(intf, bus, address,
		                               reg, length, data, rw);
	}

	return spd_raw_i2c(intf, bus, address, reg, length, data, rw);
}

int spd_read_i2c(struct platform_intf *intf, int bus,
                 int address, int reg, int length, void *data)
{
	const char module[] = "eeprom";

	if (!intf->cb->memory || !intf->cb->memory->dimm_map)
		return -1;

	/* Read info from /sys */
	if (intf->op->i2c->find_driver(intf, module)) {
		FILE *fp;
		int ret;
		char path[80];

		/* Get the data */
		snprintf(path, sizeof(path), "%s/%u-%04x/%s",
			 intf->op->i2c->sys_root, bus, address, module);
		if (!(fp = fopen(path, "r"))) {
			lprintf(LOG_DEBUG,
				"Failed to open %s\n", path);
			return -1;
		}

		fseek(fp, reg, SEEK_SET);
		ret = fread(data, 1, length, fp);
		fclose(fp);

		return ret;
	} else 	{
		return spd_raw_access(intf, bus, address, reg,
		                      length, data, SPD_READ);
	}

	return -1;
}

#if 0
int spd_write_i2c(struct platform_intf *intf,
                  int dimm, int reg, int length, const void *data)
{
	int bus, address;

	if (!intf->cb->memory->dimm_map)
		return -1;

	/* convert logical dimm map */
	bus = intf->cb->memory->dimm_map(intf, DIMM_TO_BUS, dimm);
	if (bus < 0)
		return -1;

	address = intf->cb->memory->dimm_map(intf, DIMM_TO_ADDRESS, dimm);
	if (address < 0)
		return -1;

	//FIXME: we cast data to remove the const - can we do better?
	return intf->cb->memory->spd->write(intf, bus, address, reg,
	                                    length, (void *)data);
}
#endif

int override_spd_raw_access(spd_raw_override override)
{
	spd_raw_access_override = override;
	return 0;
}

/* new_spd_device() - create a new instance of spd_device
 *
 * @intf:  platform_intf for access
 * @dimm:  Google logical dimm number to represent
 *
 * returns allocated and filled in spd_devices on success, NULL if error
 */
struct spd_device *new_spd_device(struct platform_intf *intf, int dimm)
{
	struct spd_device *spd;

	if (intf == NULL || dimm < 0) {
		return NULL;
	}

	spd = mosys_malloc(sizeof(*spd));
	spd->dimm_num = dimm;
	memset(&spd->eeprom.data[0], 0xff, SPD_MAX_LENGTH);

	if (intf->cb->memory->spd->read(intf, dimm, 0, 3,
	                                &spd->eeprom.data[0]) != 3) {
		free(spd);
		return NULL;
	}

	spd->dram_type = (enum spd_dram_type)spd->eeprom.data[2];
	spd->eeprom.length = spd_total_size(&spd->eeprom.data[0]);

	/* Invalid length. */
	if (spd->eeprom.length <= 0) {
		lperror(LOG_DEBUG, "Invalid DIMM(%d) SPD length(%d).\n",
		        dimm, spd->eeprom.length);
		free(spd);
		return NULL;
	}

	/* Fill in copy of SPD eeprom area. */
	if (intf->cb->memory->spd->read(intf, dimm, 0,
	                                spd->eeprom.length,
	                                &spd->eeprom.data[0])
			!= spd->eeprom.length) {
		lperror(LOG_DEBUG,
		        "Unable to read full contents of SPD from DIMM %d.\n",
		        dimm);
		free(spd);
		return NULL;
	}

	return spd;
}

static int find_spd_by_part_number(struct platform_intf *intf, int dimm,
				   uint8_t *spd, uint32_t num_spd)
{
	char *smbios_part_num;
	uint8_t i;
	uint8_t *ptr;
	struct smbios_table table;

	lprintf(LOG_DEBUG, "Use SMBIOS type 17 to get memory information\n");
	if (smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_DEBUG, "Can't find smbios type17\n");
		return -1;
	}
	smbios_part_num = table.string[table.data.mem_device.part_number];

	for (i = 0; i < num_spd; i++) {
		ptr = (spd + i * 256);
		if (!memcmp(smbios_part_num, ptr + 128, 18)) {
			lprintf(LOG_DEBUG, "found %x\n", i);
			return i;
		}
	}
	return -1;
}

int spd_read_from_cbfs(struct platform_intf *intf,
			int module, int reg, int num_bytes_to_read,
			uint8_t *spd, size_t fw_size, uint8_t *fw)
{
	struct cbfs_file *file;
	int spd_index = 0;
	uint32_t spd_offset, num_spd;
	uint8_t *ptr;
	int ret = 0;

	if ((file = cbfs_find("spd.bin", fw, fw_size)) == NULL) {
		ret = -1;
		goto out;
	}

	ptr = (uint8_t *)file + ntohl(file->offset);
	num_spd = ntohl(file->len) / 256;
	spd_index = find_spd_by_part_number(intf, module, ptr, num_spd);
	if (spd_index < 0) {
		ret = -1;
		goto out;
	}

	MOSYS_CHECK((spd_index * 256) + reg + num_bytes_to_read <=
							num_spd * 256);
	spd_offset = ntohl(file->offset) + (spd_index * 256);

	lprintf(LOG_DEBUG, "Using memory config %u\n", spd_index);
	memcpy(spd, (void *)file + spd_offset + reg, num_bytes_to_read);

	ret = num_bytes_to_read;
out:
	return ret;
}
