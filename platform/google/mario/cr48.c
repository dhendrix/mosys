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

#include "cr48.h"

const char *google_cr48_id_list[] = {
	"Mario",
	NULL
};

struct platform_cmd *google_cr48_sub[] = {
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_i2c,
	&cmd_memory,
//	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	&cmd_ec,
	NULL
};

static const char *hwids[] = {
	"{D3178EA2-58C9-4DD7-9676-95DBF45290BB}",
	"{9D799111-A88A-439E-9E1F-FBBB41B00A9A}",
	NULL
};

static const char *frids[] = {
	"Mario",
	NULL
};

int google_cr48_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto google_cr48_probe_exit;
	}

	if (probe_frid(frids)) {
		status = 1;
		goto google_cr48_probe_exit;
	}

	if (probe_smbios(intf, google_cr48_id_list)) {
		status = 1;
		goto google_cr48_probe_exit;
	}

google_cr48_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int google_cr48_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (google_cr48_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= google_cr48_eeprom_setup(intf);
	rc |= google_cr48_ec_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int google_cr48_destroy(struct platform_intf *intf)
{
	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb google_cr48_cb = {
	.ec		= &google_cr48_ec_cb,
	.eeprom		= &google_cr48_eeprom_cb,
	.gpio		= &google_cr48_gpio_cb,
	.memory		= &google_cr48_memory_cb,
//	.nvram		= &google_cr48_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &google_cr48_sys_cb,
	.vpd		= &google_cr48_vpd_cb,
};

struct platform_intf platform_mario = {
	.type		= PLATFORM_X86_64,
	.name		= "Mario",
	.id_list	= google_cr48_id_list,
	.sub		= google_cr48_sub,
	.cb		= &google_cr48_cb,
	.probe		= &google_cr48_probe,
	.setup_post	= &google_cr48_setup_post,
	.destroy	= &google_cr48_destroy,
};
