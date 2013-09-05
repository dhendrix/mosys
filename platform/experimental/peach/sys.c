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

#include "mosys/alloc.h"
#include "mosys/platform.h"

#include "peach.h"

#if 0
static const char *pit_get_vendor(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);

	return (const char *)ret;
}

static const char *get_version(struct platform_intf *intf)
{
	char *ret = NULL;

	switch (peach_board_config) {
	case PEACH_PIT_CONFIG_PROTO:
		ret = mosys_strdup("PROTO");
		break;
	case PEACH_PIT_CONFIG_EVT_2GB:
	case PEACH_PIT_CONFIG_EVT_4GB:
		ret = mosys_strdup("EVT");
		break;
	case PEACH_PIT_CONFIG_DVT1_2GB:
	case PEACH_PIT_CONFIG_DVT1_4GB:
	case PEACH_PIT_CONFIG_DVT2_2GB:
	case PEACH_PIT_CONFIG_DVT2_4GB:
		ret = mosys_strdup("DVT");
		break;
	case PEACH_PIT_CONFIG_PVT1_2GB:
	case PEACH_PIT_CONFIG_PVT1_4GB:
	case PEACH_PIT_CONFIG_PVT2_2GB:
	case PEACH_PIT_CONFIG_PVT2_4GB:
		ret = mosys_strdup("PVT");
		break;
	case PEACH_PIT_CONFIG_MP_2GB:
	case PEACH_PIT_CONFIG_MP_4GB:
		ret = mosys_strdup("MP");
		break;
	case PEACH_PIT_CONFIG_RSVD:
		ret = mosys_strdup("RSVD");
		break;
	default:
		ret = mosys_strdup("Unknown");
		break;
	}

	return ret;
}

struct sys_cb peach_sys_cb = {
//	.vendor		= &get_vendor,
	.name		= &get_name,
	.version	= &get_version,
};
