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

#ifndef MOSYS_LIB_SMBIOS_H__
#define MOSYS_LIB_SMBIOS_H__

#include <inttypes.h>

#include "mosys/platform.h"

#include "lib/smbios_tables.h"

struct kv_pair;

/* SMBIOS Table Types */
enum smbios_types {
	SMBIOS_TYPE_BIOS = 0,
	SMBIOS_TYPE_SYSTEM,
	SMBIOS_TYPE_BASEBOARD,
	SMBIOS_TYPE_CHASSIS,
	SMBIOS_TYPE_CPU,
	SMBIOS_TYPE_MEMCTRL,
	SMBIOS_TYPE_MEMMODULE,
	SMBIOS_TYPE_CACHE,
	SMBIOS_TYPE_PORT,
	SMBIOS_TYPE_SLOT,
	SMBIOS_TYPE_ONBOARD,
	SMBIOS_TYPE_OEM,
	SMBIOS_TYPE_SYSCFG,
	SMBIOS_TYPE_BIOSLANG,
	SMBIOS_TYPE_GROUP,
	SMBIOS_TYPE_LOG,
	SMBIOS_TYPE_MEMARRAY,
	SMBIOS_TYPE_MEMORY,
	SMBIOS_TYPE_MEMERR32,
	SMBIOS_TYPE_MEMA_MAP,
	SMBIOS_TYPE_MEMD_MAP,
	SMBIOS_TYPE_MOUSE,
	SMBIOS_TYPE_BATTERY,
	SMBIOS_TYPE_SYSRESET,
	SMBIOS_TYPE_HWSEC,
	SMBIOS_TYPE_POWER,
	SMBIOS_TYPE_VOLTAGE,
	SMBIOS_TYPE_FAN,
	SMBIOS_TYPE_TEMP,
	SMBIOS_TYPE_CURRENT,
	SMBIOS_TYPE_OOB,
	SMBIOS_TYPE_BIS,
	SMBIOS_TYPE_SYSBOOT,
	SMBIOS_TYPE_MEMERR64,
	SMBIOS_TYPE_MGMTDEV,
	SMBIOS_TYPE_MGMTCMP,
	SMBIOS_TYPE_MGMTTHRLD,
	SMBIOS_TYPE_MEMCHAN,
	SMBIOS_TYPE_IPMI,
	SMBIOS_TYPE_PWRSUPPLY,
	SMBIOS_TYPE_INACTIVE = 126,
	SMBIOS_TYPE_END = 127,
};

/* SMBIOS platform information callbacks */
extern struct smbios_cb smbios_sysinfo_cb;

extern int smbios_dimm_count(struct platform_intf *intf);

extern int smbios_dimm_speed(struct platform_intf *intf,
		     int dimm, struct kv_pair *kv);

/* SMBIOS main API. */
extern int smbios_find_table(struct platform_intf *intf,
                             enum smbios_types type,
                             int instance, struct smbios_table *table,
                             unsigned int baseaddr, unsigned int len);
extern char *smbios_find_string(struct platform_intf *intf,
                                enum smbios_types type, int number,
                             unsigned int baseaddr, unsigned int len);
extern int smbios_find_entry(struct platform_intf *intf,
			     struct smbios_entry *entry,
                             unsigned long int baseaddr,
                             unsigned long int len);

extern char *smbios_bios_get_vendor(struct platform_intf *intf);
extern char *smbios_sysinfo_get_vendor(struct platform_intf *intf);
extern char *smbios_sysinfo_get_name(struct platform_intf *intf);
extern char *smbios_sysinfo_get_version(struct platform_intf *intf);
extern char *smbios_sysinfo_get_family(struct platform_intf *intf);
extern char *smbios_sysinfo_get_sku(struct platform_intf *intf);

#endif /* MOSYS_LIB_SMBIOS_H__ */
