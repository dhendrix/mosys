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
	EEPROM_UNKNOWN	= 0,
	EEPROM_RD,
	EEPROM_WR,
	EEPROM_EVENTLOG,	/* has an eventlog */
	EEPROM_FMAP,		/* has an FMAP blob */
	EEPROM_VERBOSE_ONLY,
};

#define EEPROM_FLAG_RD			1 << EEPROM_RD
#define EEPROM_FLAG_WR			1 << EEPROM_WR
#define EEPROM_FLAG_RDWR		(1 << EEPROM_RD) | (1 << EEPROM_WR)
#define EEPROM_FLAG_EVENTLOG		1 << EEPROM_EVENTLOG
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

/* eeprom_region contains attributes used to assist library functions */
struct eeprom_region {
	const char *name;	/* may be something found in an fmap */
	int handle;		/* handle for this instance */
	off_t offset;
	size_t len;
	enum eeprom_flag_types flag;
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
	int (*setup)(struct platform_intf *intf, struct eeprom *eeprom);
	void *priv;

	/*
	 * Regions of interest may be described with attributes to assist
	 * library functions in finding and processing them.
	 */
	struct eeprom_region *regions;
};

extern int eeprom_mmio_read(struct platform_intf *intf, struct eeprom *eeprom,
                            unsigned int offset, unsigned int len, void *data);

/*
 * eeprom_get_fmap - return a newly allocated copy of EEPROM's FMAP
 *
 * @intf:	platform interface
 * @eeprom:	eeprom structure
 *
 * This function will attempt to find an FMAP data structure embedded in an
 * EEPROM image and will copy it to a newly allocated buffer if found.
 *
 * returns a newly allocated struct fmap if successful
 * returns NULL to indicate error or if no fmap is found
 */
extern struct fmap *eeprom_get_fmap(struct platform_intf *intf,
                                    struct eeprom *eeprom);

#endif /* MOSYS_LIB_EEPROM_H__ */
