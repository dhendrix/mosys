/*
 * Copyright (C) 2010 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <unistd.h>

#include "mosys/platform.h"
#include "mosys/common.h"
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/eeprom_enet.h"

#define HP_Z600_FIRMWARE_SIZE	(2048 * 1024)
static struct eeprom_dev z600_host_firmware = {
	.size	= HP_Z600_FIRMWARE_SIZE,
	.read	= eeprom_mmio_read,
};

static struct eeprom_dev z600_eth0 = {
	.size	= 128 * 1024,
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
