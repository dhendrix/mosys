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

#ifndef MOSYS_COMMAND_LIST_H__
#define MOSYS_COMMAND_LIST_H__

struct platform_cmd;

extern struct platform_cmd cmd_platform;
extern struct platform_cmd cmd_i2c;
extern struct platform_cmd cmd_gpio;
extern struct platform_cmd cmd_smbios;
extern struct platform_cmd cmd_memory;
extern struct platform_cmd cmd_cpu;
extern struct platform_cmd cmd_sensor;
extern struct platform_cmd cmd_eventlog_smbios;
extern struct platform_cmd cmd_bootnum;
extern struct platform_cmd cmd_flash;
extern struct platform_cmd cmd_nvram;
extern struct platform_cmd cmd_mce;
extern struct platform_cmd cmd_ht;
extern struct platform_cmd cmd_edac;
extern struct platform_cmd cmd_eeprom;
extern struct platform_cmd cmd_vpd;
extern struct platform_cmd cmd_ec;
//extern struct platform_cmd cmd_fru;

#endif /* MOSYS_COMMAND_LIST_H__ */
