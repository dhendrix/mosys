/*
 * Copyright 2013, Google Inc.
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

#include <valstr.h>

#include "mosys/alloc.h"
#include "mosys/platform.h"

#include "lib/smbios.h"

#include "drivers/google/cros_ec.h"

/*TODO: To be added when board versions are available
 * static struct valstr glados_board_version[] = {
	{ 0, "Proto1/1.5" },
	{ 1, "Proto2" },
	{ },
};

static const char *glados_get_version(struct platform_intf *intf)
{
	return mosys_strdup(val2str(cros_ec_board_version(intf, intf->cb->ec),
				glados_board_version));
}*/

static char *glados_get_name(struct platform_intf *intf)
{
	return mosys_strdup(intf->name);
}

struct sys_cb glados_sys_cb = {
	/*.version		= &glados_get_version,*/
	.vendor			= &smbios_sysinfo_get_vendor,
	.name			= &glados_get_name,
	.family			= &smbios_sysinfo_get_family,
	.firmware_vendor	= &smbios_bios_get_vendor,
};
