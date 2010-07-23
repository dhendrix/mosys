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
 *
 */

#ifndef MOSYS_LIB_EEPROM_H__
#define MOSYS_LIB_EEPROM_H__

#include "intf/i2c.h"
#include "intf/mmio.h"
#include "intf/pci.h"

struct fmap;

enum eeprom_type {
	EEPROM_RAW,		/* arbitrary contents */
	EEPROM_SPD,		/* follows a JEDEC SPD spec */
	EEPROM_FRU,		/* follows FRU spec */
	EEPROM_ENET,		/* ethernet */
	EEPROM_FW,		/* firmware */
};

#define EEPROM_TYPE_RAW			1 << EEPROM_RAW
#define EEPROM_TYPE_SPD			1 << EEPROM_SPD
#define EEPROM_TYPE_FRU			1 << EEPROM_FRU
#define EEPROM_TYPE_ENET		1 << EEPROM_ENET
#define EEPROM_TYPE_FW			1 << EEPROM_FW

enum eeprom_flag_types {
	EEPROM_RD,
	EEPROM_WR,
	EEPROM_FMAP,		/* has an FMAP blob */
	EEPROM_VERBOSE_ONLY,
};

#define EEPROM_FLAG_RD			1 << EEPROM_RD
#define EEPROM_FLAG_WR			1 << EEPROM_WR
#define EEPROM_FLAG_RDWR		(1 << EEPROM_RD) | (1 << EEPROM_WR)
#define EEPROM_FLAG_FMAP		1 << EEPROM_FMAP
#define EEPROM_FLAG_VERBOSE_ONLY	1 << EEPROM_VERBOSE_ONLY

struct eeprom;
struct eeprom_dev {
	/*
	 * size  -  return size of eeprom (in bytes)
	 *
	 * @intf:	platform interface
	 * @eeprom:	eeprom interface
	 *
	 * returns <0 to indicate error
	 */
	size_t (*size)(struct platform_intf *intf,
	               struct eeprom *eeprom);

	/*
	 * read  -  read from eeprom
	 *
	 * @intf:	platform interface
	 * @eeprom:	eeprom interface
	 * @offset:	data offset
	 * @len:	length of data
	 * @data:	data buffer
	 *
	 * returns the number of bytes read if successful
	 * returns <0 to indicate error
	 */
	int (*read)(struct platform_intf *intf,
		    struct eeprom *eeprom,
		    unsigned int offset,
		    unsigned int len,
		    void *data);

	/*
	 * write  -  write to eeprom
	 *
	 * @intf:	platform interface
	 * @eeprom:	eeprom interface
	 * @offset:	data offset
	 * @len:	length of data
	 * @data:	data buffer
	 *
	 * returns the number of bytes written if successful
	 * returns <0 to indicate error
	 */
	int (*write)(struct platform_intf *intf,
		     struct eeprom *eeprom,
		     unsigned int offset,
		     unsigned int len,
		     void *data);

	/*
	 * get_map  -  retrieve flash map
	 *
	 * @intf:	platform interface
	 * @eeprom:	eeprom interface
	 *
	 * returns newly allocated flash map if successful
	 * returns NULL to indicate error
	 */
	struct fmap *(*get_map)(struct platform_intf *intf,
	                        struct eeprom *eeprom);
};

/* high-level eeprom interface (includes name, topology info, etc) */
struct eeprom {
	const char *name;
	enum eeprom_type type;
	union {
		struct i2c_addr i2c;
		struct pci_addr pci;
		unsigned long long mmio;
	} addr;
	struct eeprom_dev *device;
	uint8_t flags;
	void *priv;
};

extern int eeprom_mmio_read(struct platform_intf *intf, struct eeprom *eeprom,
                            unsigned int offset, unsigned int len, void *data);

#endif /* MOSYS_LIB_EEPROM_H__ */
