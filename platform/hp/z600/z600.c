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

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/probe.h"
#include "lib/smbios.h"

#include "z600.h"

static const char *probed_platform_id;

const char *hp_z600_id_list[] = {
	"HP Z600 Workstation",
	NULL
};

struct platform_cmd *platform_hp_z600_sub[] = {
	&cmd_eeprom,
	&cmd_platform,
	&cmd_smbios,
	NULL
};

int hp_z600_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_smbios(intf, hp_z600_id_list)) {
		status = 1;
		goto z600_pinetrail_probe_exit;
	}

z600_pinetrail_probe_exit:
	probed = 1;
	return status;
}

static int hp_z600_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	return 0;
}

struct platform_cb hp_z600_cb = {
	.eeprom		= &hp_z600_eeprom_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &hp_z600_sys_cb,
};

struct platform_intf platform_hp_z600 = {
	.type		= PLATFORM_X86_64,
	.name		= "HP Z600 Workstation",
	.id_list	= hp_z600_id_list,
	.sub		= platform_hp_z600_sub,
	.cb		= &hp_z600_cb,
	.probe		= hp_z600_probe,
	.destroy	= &hp_z600_destroy,
};
