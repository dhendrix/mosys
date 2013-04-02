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

#include "spring.h"

#if 0
static const char *spring_get_vendor(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *spring_get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);

	return (const char *)ret;
}

#if 0
static const char *spring_get_family(struct platform_intf *intf)
{
	/* FIXME: add then when it becomes public */
}
#endif

static const char *spring_get_version(struct platform_intf *intf)
{
	char *ret = NULL;

	switch (spring_board_config) {
	case SPRING_CONFIG_PROTO:
		ret = mosys_strdup("PROTO");
		break;
	case SPRING_CONFIG_EVT_NANYA:
	case SPRING_CONFIG_EVT_MICRON:
		ret = mosys_strdup("EVT");
		break;
	case SPRING_CONFIG_DVT_NANYA:
	case SPRING_CONFIG_DVT_MICRON:
		ret = mosys_strdup("DVT");
		break;
	case SPRING_CONFIG_PVT_NANYA:
	case SPRING_CONFIG_PVT_MICRON:
		ret = mosys_strdup("PVT");
		break;
	case SPRING_CONFIG_MP_NANYA:
	case SPRING_CONFIG_MP_MICRON:
		ret = mosys_strdup("MP");
		break;
	case SPRING_CONFIG_RSVD:
		ret = mosys_strdup("RSVD");
		break;
	default:
		ret = mosys_strdup("Unknown");
		break;
	}

	return ret;
}

struct sys_cb spring_sys_cb = {
//	.vendor		= &spring_get_vendor,
	.name		= &spring_get_name,
//	.family		= &spring_get_family,
	.version	= &spring_get_version,
};
