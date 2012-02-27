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

#include <stdlib.h>

/* 
 * FIXME: This is too x86-centric right now. We'll need to be smarter
 * about making sane choices for interfaces and commands on other
 * architectures.
 */

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/log.h"

#include "lib/smbios.h"

#include "x86.h"

/* FIXME: command/command_list.h is included magically via Makefile */
const char *default_x86_id_list[] = {
	NULL
};

struct platform_cmd *platform_default_x86_sub[] = {
	&cmd_eeprom,
	&cmd_smbios,
	&cmd_platform,
	NULL
};

struct sys_cb default_x86_sys_cb;

struct platform_cb default_x86_cb = {
	.eeprom = &default_x86_eeprom_cb,
	.smbios = &smbios_sysinfo_cb,
	.sys	= &default_x86_sys_cb,
};

int default_x86_probe(struct platform_intf *intf)
{
	return 1;
}

static int default_x86_setup_post(struct platform_intf *intf)
{
	default_x86_eeprom_setup(intf);

	return 0;
}

struct platform_intf platform_default_x86 = {
	.type		= PLATFORM_X86,
	.name		= "Default",
	.probe		= default_x86_probe,
	.id_list	= default_x86_id_list,
	.sub		= platform_default_x86_sub,
	.cb		= &default_x86_cb,
	.setup_post	= &default_x86_setup_post,
};
