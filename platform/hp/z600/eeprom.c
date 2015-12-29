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

#include <stdio.h>
#include <unistd.h>

#include "mosys/platform.h"
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/eeprom_enet.h"

#define HP_Z600_FIRMWARE_SIZE		(2048 * 1024)
#define HP_Z600_ETH0_EEPROM_SIZE	(128 * 1024)

static int z600_fw_size(struct platform_intf *intf,
                           struct eeprom *eeprom)
{
	return HP_Z600_FIRMWARE_SIZE;
}

static struct eeprom_dev z600_host_firmware = {
	.size	= z600_fw_size,
	.read	= eeprom_mmio_read,
};

static int z600_eth0_size(struct platform_intf *intf, struct eeprom *eeprom)
{
	return HP_Z600_ETH0_EEPROM_SIZE;
}

static struct eeprom_dev z600_eth0 = {
	.size	= z600_eth0_size,
};

static struct eeprom z600_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		.addr.mmio	= 0xffffffff - HP_Z600_FIRMWARE_SIZE + 1,
		.device		= &z600_host_firmware,
		.flags		= EEPROM_FLAG_RDWR,
	},
	{
		.name		= "eth0",
		.type		= EEPROM_TYPE_ENET,
		.addr.pci	= { 1, 0, 0 },
		.device		= &z600_eth0,
		.flags		= EEPROM_FLAG_RDWR,
	},
	{ 0 },
};

static int z600_eeprom_enet_read(struct platform_intf *intf,
                                 int argc, char **argv) {
	int i, num_devices;
	unsigned int mode = 0;

	/* Get serial EEPROM data only for now */
	mode |= EEPROM_ENET_GEEPROM;
	
	num_devices = sizeof(z600_eeproms) / sizeof(z600_eeproms[0]);
	for (i = 0; i < num_devices; i++) {
		if (z600_eeproms[i].type != EEPROM_TYPE_ENET)
			continue;

		/* If an interface was specified, print only its information */
		if (argc != 0) {
			if (strncmp(argv[0], z600_eeproms[i].name,
					strlen(z600_eeproms[i].name)) != 0)
				continue;
		}
		eeprom_enet_doit(intf, z600_eeproms[i].name, mode);
	}

	return 0;
}
#if 0
static int z600_eeprom_enet_read(struct platform_intf *intf,
                                 int argc, char **argv) {
	int i, num_devices;
	unsigned int mode = 0;

	/* Get serial EEPROM data only for now */
	mode |= EEPROM_ENET_GEEPROM;
	
	num_devices = sizeof(enet_devices) / sizeof(enet_devices[0]);
	for (i = 0; i < num_devices; i++) {
		/* If an interface was specified, print only its information */
		if (argc != 0) {
			if (strncmp(argv[0], enet_devices[i].devname,
					strlen(enet_devices[i].devname)) != 0)
				continue;
		}
		eeprom_enet_doit(intf, enet_devices[i].devname, mode);
	}

	return 0;
}
#endif

static struct eeprom_enet_cb hp_z600_eeprom_enet_cb = {
	.read	= z600_eeprom_enet_read,
};

struct eeprom_cb hp_z600_eeprom_cb = {
	.enet		= &hp_z600_eeprom_enet_cb,
	.eeprom_list	= z600_eeproms,
};
