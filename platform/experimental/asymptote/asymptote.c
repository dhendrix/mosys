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
#include <unistd.h>

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/math.h"
#include "lib/probe.h"

#include "asymptote.h"

static const char *probed_platform_id;

const char *asymptote_tegra2_id_list[] = {
	"Asymptote",
	NULL
};

struct platform_cmd *asymptote_tegra2_sub[] = {
	&cmd_platform,
	NULL
};

int asymptote_tegra2_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;

	if (probed)
		return status;

	for (id = asymptote_tegra2_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto asymptote_tegra2_probe_exit;
		}
	}

asymptote_tegra2_probe_exit:
	probed = 1;
	return status;
}

static int asymptote_tegra2_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);
	return 0;
}

struct platform_cb asymptote_tegra2_cb = {
	.sys 		= &asymptote_tegra2_sys_cb,
};

struct platform_intf platform_asymptote_tegra2 = {
	.type		= PLATFORM_ARMV7,
	.name		= "Asymptote",
	.id_list	= asymptote_tegra2_id_list,
	.sub		= asymptote_tegra2_sub,
	.cb		= &asymptote_tegra2_cb,
	.probe		= &asymptote_tegra2_probe,
	.destroy	= &asymptote_tegra2_destroy,
};
