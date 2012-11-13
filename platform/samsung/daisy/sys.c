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

#include "mosys/alloc.h"
#include "mosys/platform.h"

#include "lib/probe.h"

#include "daisy.h"

#if 0
static const char *daisy_get_vendor(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *daisy_get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);

	return (const char *)ret;
}

#if 0
static const char *daisy_get_family(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *daisy_get_version(struct platform_intf *intf)
{
	char *ret = NULL;

	switch (board_config) {
	case SNOW_CONFIG_ELPIDA_EVT:
	case SNOW_CONFIG_SAMSUNG_EVT:
		ret = mosys_strdup("EVT");
		break;
	case SNOW_CONFIG_ELPIDA_DVT:
	case SNOW_CONFIG_SAMSUNG_DVT:
		ret = mosys_strdup("DVT");
		break;
	case SNOW_CONFIG_ELPIDA_PVT:
	case SNOW_CONFIG_SAMSUNG_PVT:
		ret = mosys_strdup("PVT");
		break;
	case SNOW_CONFIG_ELPIDA_PVT2:
	case SNOW_CONFIG_SAMSUNG_PVT2:
		ret = mosys_strdup("PVT2");
		break;
	case SNOW_CONFIG_ELPIDA_MP:
	case SNOW_CONFIG_SAMSUNG_MP:
		ret = mosys_strdup("MP");
		break;
	case SNOW_CONFIG_RSVD:
		ret = mosys_strdup("RSVD");
		break;
	default:
		ret = mosys_strdup("Unknown");
		break;
	}

	return ret;
}

struct sys_cb daisy_sys_cb = {
//	.vendor		= &daisy_get_vendor,
	.name		= &daisy_get_name,
//	.family		= &daisy_get_family,
	.version	= &daisy_get_version,
};
