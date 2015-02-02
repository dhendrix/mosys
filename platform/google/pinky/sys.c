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

#include "lib/probe.h"
#include "lib/fdt.h"

/* Pinky uses device-tree compatible typle: google,<family>-<name>-rev<N>,
 * ie "google,veyron-pinky-rev0" */

static const char *pinky_get_version(struct platform_intf *intf)
{
	uint32_t board_id;
	char board_id_str[16];

	board_id = fdt_get_board_id();
	if (board_id == 0xffffffff)
		return NULL;

	snprintf(board_id_str, sizeof(board_id_str), "rev%u", board_id);
	return mosys_strdup(board_id_str);
}

static const char *pinky_get_vendor(struct platform_intf *intf)
{
	/* FIXME: implement this */
	return mosys_strdup("Unknown");
}

static const char *pinky_get_name(struct platform_intf *intf)
{
	return mosys_strdup(intf->name);
}

struct sys_cb pinky_sys_cb = {
	.vendor			= &pinky_get_vendor,
	.name			= &pinky_get_name,
	.version		= &pinky_get_version,
};
