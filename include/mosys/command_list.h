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
 *
 */

#ifndef MOSYS_COMMAND_LIST_H__
#define MOSYS_COMMAND_LIST_H__

struct platform_cmd;

extern struct platform_cmd cmd_platform;
extern struct platform_cmd cmd_i2c;
extern struct platform_cmd cmd_gpio;
extern struct platform_cmd cmd_hid;
extern struct platform_cmd cmd_smbios;
extern struct platform_cmd cmd_memory;
extern struct platform_cmd cmd_cpu;
extern struct platform_cmd cmd_sensor;
extern struct platform_cmd cmd_eventlog;
extern struct platform_cmd cmd_bootnum;
extern struct platform_cmd cmd_flash;
extern struct platform_cmd cmd_nvram;
extern struct platform_cmd cmd_mce;
extern struct platform_cmd cmd_ht;
extern struct platform_cmd cmd_edac;
extern struct platform_cmd cmd_eeprom;
extern struct platform_cmd cmd_vpd;
extern struct platform_cmd cmd_ec;
extern struct platform_cmd cmd_sh;
extern struct platform_cmd cmd_pd;
extern struct platform_cmd cmd_fp;
extern struct platform_cmd cmd_battery;
extern struct platform_cmd cmd_storage;
extern struct platform_cmd cmd_psu;
//extern struct platform_cmd cmd_fru;

#endif /* MOSYS_COMMAND_LIST_H__ */
