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

#include "drivers/intel/series6.h"

#include "mosys/alloc.h"
#include "mosys/platform.h"

#include "lib/smbios.h"

static const char *butterfly_get_vendor(struct platform_intf *intf)
{
	if (intf->cb && intf->cb->smbios)
		return intf->cb->smbios->system_vendor(intf);
	else
		return NULL;
}

static const char *butterfly_get_name(struct platform_intf *intf)
{
	return mosys_strdup(intf->name);
}

static const char *butterfly_get_family(struct platform_intf *intf)
{
	if (intf->cb && intf->cb->smbios)
		return intf->cb->smbios->system_family(intf);
	else
		return NULL;
}

static int butterfly_reset(struct platform_intf *intf)
{
	return series6_global_reset(intf);
}

struct sys_cb butterfly_sys_cb = {
	.vendor		= &butterfly_get_vendor,
	.name		= &butterfly_get_name,
	.family		= &butterfly_get_family,
	.reset		= &butterfly_reset,
};
