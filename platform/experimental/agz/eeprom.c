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
#include "mosys/log.h"

#include "lib/eeprom.h"
#include "lib/eeprom_enet.h"

static struct eeprom agz_pinetrail_eeproms[] = {
	{
		.name		= "host_firmware",
		.type		= EEPROM_TYPE_FW,
		/* FIXME: add proper address stuff here */
		.flags		= EEPROM_FLAG_RDWR,
	},
	{
		.name		= "ec_firmware",
		.type		= EEPROM_TYPE_FW,
		/* FIXME: add proper address stuff here */
		.flags		= EEPROM_FLAG_RDWR,
	},
	{ 0 },
};

struct eeprom_cb agz_pinetrail_eeprom_cb = {
	.eeprom_list	= agz_pinetrail_eeproms,
};
