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

#ifndef MOSYS_LIB_SMBIOS_H__
#define MOSYS_LIB_SMBIOS_H__

#include <inttypes.h>

#include "smbios_tables.h"

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

/* SMBIOS main API. */
extern int smbios_find_table(struct platform_intf *intf,
                             enum smbios_types type,
                             int instance, struct smbios_table *table,
                             unsigned int baseaddr, unsigned int len);
extern char *smbios_find_string(struct platform_intf *intf,
                                enum smbios_types type, int number,
                             unsigned int baseaddr, unsigned int len);
/* SMBIOS Event Log */
extern int smbios_find_entry(struct platform_intf *intf,
			     struct smbios_entry *entry,
                             unsigned long int baseaddr,
                             unsigned long int len);
extern struct smbios_eventlog_iterator *smbios_new_eventlog_iterator(
    struct platform_intf *intf, struct smbios_table_log *elog_table);
extern void smbios_free_eventlog_iterator(
    struct smbios_eventlog_iterator *elog_iter);
extern int smbios_eventlog_iterator_reset(
    struct smbios_eventlog_iterator *elog_iter);
extern struct smbios_log_entry *smbios_eventlog_get_next_entry(
    struct smbios_eventlog_iterator *elog_iter);
extern struct smbios_log_entry *smbios_eventlog_get_current_entry(
    struct smbios_eventlog_iterator *elog_iter);
extern void *smbios_eventlog_get_header(
    struct smbios_eventlog_iterator *elog_iter);
extern const char *smbios_get_event_type_string(struct smbios_log_entry *entry);

#endif /* MOSYS_LIB_SMBIOS_H__ */
