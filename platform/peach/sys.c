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

static char *get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);

	return (const char *)ret;
}

static char *get_version(struct platform_intf *intf)
{
	char *ret = NULL;

	switch (peach_board_config) {
	case PEACH_PIT_CONFIG_REV_0_0:
		ret = mosys_strdup("EVT_OLD");
		break;
	case PEACH_PIT_CONFIG_REV_3_0:
	case PEACH_PIT_CONFIG_REV_4_0:
		ret = mosys_strdup("DVT_OLD");
		break;
	case PEACH_PIT_CONFIG_REV_5_0:
	case PEACH_PIT_CONFIG_REV_6_0:
	case PEACH_PIT_CONFIG_REV_7_0:
	case PEACH_PIT_CONFIG_REV_7_2:
		ret = mosys_strdup("PVT_OLD");
		break;
	case PEACH_PI_CONFIG_REV_8_4:
		ret = mosys_strdup("EVT");
		break;
	case PEACH_PIT_CONFIG_REV_9_0:
	case PEACH_PIT_CONFIG_REV_9_2:
	case PEACH_PI_CONFIG_REV_9_4:
		ret = mosys_strdup("DVT1");
		break;
	case PEACH_PIT_CONFIG_REV_A_0:
	case PEACH_PIT_CONFIG_REV_A_2:
	case PEACH_PI_CONFIG_REV_A_6:
		ret = mosys_strdup("DVT2");
		break;
	case PEACH_PIT_CONFIG_REV_B_0:
	case PEACH_PIT_CONFIG_REV_B_2:
	case PEACH_PI_CONFIG_REV_B_6:
		ret = mosys_strdup("PVT1");
		break;
	case PEACH_PIT_CONFIG_REV_C_0:
	case PEACH_PIT_CONFIG_REV_C_2:
	case PEACH_PI_CONFIG_REV_C_6:
		ret = mosys_strdup("PVT2");
		break;
	case PEACH_PIT_CONFIG_REV_D_0:
	case PEACH_PIT_CONFIG_REV_D_2:
	case PEACH_PI_CONFIG_REV_D_6:
		ret = mosys_strdup("PVT3");
		break;
	case PEACH_PIT_CONFIG_REV_E_0:
	case PEACH_PIT_CONFIG_REV_E_2:
	case PEACH_PI_CONFIG_REV_E_6:
		ret = mosys_strdup("MP");
		break;
	case PEACH_PIT_CONFIG_RSVD:
	case PEACH_PI_CONFIG_RSVD:
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
