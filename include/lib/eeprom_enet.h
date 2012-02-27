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
