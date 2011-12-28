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
#ifndef HP_Z600_H__
#define HP_Z600_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define HP_Z600_CPU_COUNT		2
#define HP_Z600_DIMM_COUNT		6
#define HP_Z600_DIMM_PER_CHANNEL	3

struct dimm_count_db {
	int type;
	uint32_t count[HP_Z600_DIMM_COUNT];
};

/* platform callbacks */
extern struct eeprom_cb hp_z600_eeprom_cb;	/* eeprom.c */
extern struct sys_cb hp_z600_sys_cb;		/* sys.c */
//extern struct memory_cb hp_z600_memory_cb;	/* memory.c */
//extern struct flash_cb hp_z600_flash_cb;	/* flash.c */
//extern struct nvram_cb hp_z600_nvram_cb;	/* nvram.c */

#endif /* HP_Z600_H__ */
