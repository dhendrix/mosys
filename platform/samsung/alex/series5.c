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

#include "mosys/alloc.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/probe.h"
#include "lib/smbios.h"
#include "lib/vpd.h"

#include "series5.h"

static const char *probed_platform_id;

const char *samsung_series5_id_list[] = {
	"Alex",
	NULL
};

struct platform_cmd *samsung_series5_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_i2c,
	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static const char *frids[] = {
	"Alex",
	"Google_Alex",
	NULL
};

int samsung_series5_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_frid(frids)) {
		status = 1;
		goto samsung_series5_probe_exit;
	}

	if (probe_smbios(intf, samsung_series5_id_list)) {
		status = 1;
		goto samsung_series5_probe_exit;
	}

samsung_series5_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int samsung_series5_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (samsung_series5_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	if (samsung_series5_ec_setup(intf) < 0) {
		lprintf(LOG_WARNING, "Non-fatal error: Failed to setup "
		                     "EC callbacks.\n");

	}
	rc |= samsung_series5_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int samsung_series5_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	samsung_series5_ec_destroy(intf);
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb samsung_series5_cb = {
	.eeprom		= &samsung_series5_eeprom_cb,
	.gpio		= &samsung_series5_gpio_cb,
	.legacy_ec	= &samsung_series5_ec_cb,
	.memory		= &samsung_series5_memory_cb,
//	.nvram		= &samsung_series5_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &samsung_series5_sys_cb,
	.vpd		= &samsung_series5_vpd_cb,
};

struct platform_intf platform_alex = {
	.type		= PLATFORM_X86_64,
	.name		= "Alex",
	.id_list	= samsung_series5_id_list,
	.sub		= samsung_series5_sub,
	.cb		= &samsung_series5_cb,
	.probe		= &samsung_series5_probe,
	.setup_post	= &samsung_series5_setup_post,
	.destroy	= &samsung_series5_destroy,
};
