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

#ifndef MOSYS_LIB_EEPROM_ENET_H__
#define MOSYS_LIB_EEPROM_ENET_H__

#include <stdint.h>
//#include <linux/if.h>
//#include <linux/ethtool.h>

struct platform_intf;
extern int eeprom_enet_doit(struct platform_intf *intf,
                            const char *devname, unsigned int mode);

enum EEPROM_ENET_MODES {
#if 0
	EEPROM_ENET_GSET	= 1 << 0,
	EEPROM_ENET_SSET	= 1 << 1,
	EEPROM_ENET_GDRV	= 1 << 2,
	EEPROM_ENET_GREGS	= 1 << 3,
	EEPROM_ENET_NWAY_RST	= 1 << 4,
#endif
	EEPROM_ENET_GEEPROM	= 1 << 5,
#if 0
	EEPROM_ENET_SEEPROM	= 1 << 6,
	EEPROM_ENET_TEST	= 1 << 7,
	EEPROM_ENET_PHYS_ID	= 1 << 8,
	EEPROM_ENET_GPAUSE	= 1 << 9,
	EEPROM_ENET_SPAUSE	= 1 << 10,
	EEPROM_ENET_GCOALESCE	= 1 << 11,
	EEPROM_ENET_SCOALESCE	= 1 << 12,
	EEPROM_ENET_GRING	= 1 << 13,
	EEPROM_ENET_SRING	= 1 << 14,
	EEPROM_ENET_GOFFLOAD	= 1 << 15,
	EEPROM_ENET_SOFFLOAD	= 1 << 16,
	EEPROM_ENET_GSTATS	= 1 << 17,
#endif
};

#endif /* MOSYS_LIB_EEPROM_ENET_H__ */
