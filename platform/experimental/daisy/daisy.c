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

#include "lib/probe.h"

#include "daisy.h"

static const char *probed_platform_id;

const char *daisy_id_list[] = {
	"Daisy",
	"SMDK5250",
	"Snow",
	NULL
};

struct platform_cmd *daisy_sub[] = {
	&cmd_platform,
	NULL
};

#if 0
static const char *hwids[] = {
	NULL
};
#endif

int daisy_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;

	if (probed)
		return status;

	for (id = daisy_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto daisy_probe_exit;
		}

		if (probe_cmdline(*id, 0) == 1) {
			status = 1;
			goto daisy_probe_exit;
		}
	}

#if 0
	if (probe_hwid(hwids)) {
		status = 1;
		goto daisy_probe_exit;
	}
#endif

daisy_probe_exit:
	probed = 1;
	return status;
}

static int daisy_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	return 0;
}

struct platform_cb daisy_cb = {
	.sys 		= &daisy_sys_cb,
};

struct platform_intf platform_daisy = {
	.type		= PLATFORM_ARMV7,
	.name		= "Daisy",
	.id_list	= daisy_id_list,
	.sub		= daisy_sub,
	.cb		= &daisy_cb,
	.probe		= &daisy_probe,
	.destroy	= &daisy_destroy,
};
