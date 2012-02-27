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
//#include "lib/vpd.h"

#include "seaboard.h"

const char *seaboard_tegra2_id_list[] = {
	"Seaboard",
	NULL
};

struct platform_cmd *seaboard_tegra2_sub[] = {
	&cmd_eeprom,
//	&cmd_gpio,
//	&cmd_i2c,
//	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
//	&cmd_vpd,
	NULL
};

#if 0
static const char *hwids[] = {
	NULL
};
#endif

int seaboard_tegra2_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;

	if (probed)
		return status;

	for (id = seaboard_tegra2_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto seaboard_tegra2_probe_exit;
		}
	}

#if 0
	if (probe_hwid(hwids)) {
		status = 1;
		goto seaboard_tegra2_probe_exit;
	}
#endif

seaboard_tegra2_probe_exit:
	probed = 1;
	return status;
}

#if 0
/* late setup routine; not critical to core functionality */
static int seaboard_tegra2_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
//	rc |= seaboard_tegra2_vpd_setup(intf);
	if (seaboard_tegra2_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= seaboard_tegra2_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}
#endif

static int seaboard_tegra2_destroy(struct platform_intf *intf)
{
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb seaboard_tegra2_cb = {
//	.eeprom		= &seaboard_tegra2_eeprom_cb,
//	.gpio		= &seaboard_tegra2_gpio_cb,
//	.memory		= &seaboard_tegra2_memory_cb,
//	.nvram		= &seaboard_tegra2_nvram_cb,
	.sys 		= &seaboard_tegra2_sys_cb,
//	.vpd		= &seaboard_tegra2_vpd_cb,
};

struct platform_intf platform_seaboard_tegra2 = {
	.type		= PLATFORM_ARMV7,
	.name		= "Seaboard",
	.id_list	= seaboard_tegra2_id_list,
	.sub		= seaboard_tegra2_sub,
	.cb		= &seaboard_tegra2_cb,
	.probe		= &seaboard_tegra2_probe,
//	.setup_post	= &seaboard_tegra2_setup_post,
	.destroy	= &seaboard_tegra2_destroy,
};
