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

#include "kiev.h"

static const char *probed_platform_id;

const char *kiev_id_list[] = {
	"Kiev",
	NULL
};

struct platform_cmd *kiev_sub[] = {
	&cmd_eeprom,
//	&cmd_gpio,
//	&cmd_i2c,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_sensor,
	&cmd_smbios,
	&cmd_vpd,
	NULL
};

static const char *hwids[] = {
	"X86 KIEV",
};

static const char *frids[] = {
	"Kiev",
	NULL
};

int kiev_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto kiev_probe_exit;
	}

	if (probe_frid(frids)) {
		status = 1;
		goto kiev_probe_exit;
	}

	if (probe_smbios(intf, kiev_id_list)) {
		status = 1;
		goto kiev_probe_exit;
	}

kiev_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int kiev_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (kiev_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= kiev_eeprom_setup(intf);
	rc |= kiev_superio_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int kiev_destroy(struct platform_intf *intf)
{
	kiev_superio_destroy(intf);

	if (probed_platform_id)
		free((char *)probed_platform_id);

	/* FIXME: unmap vpd stuff */
	return 0;
}

struct platform_cb kiev_cb = {
	.eeprom		= &kiev_eeprom_cb,
//	.gpio		= &kiev_gpio_cb,
	.memory		= &kiev_memory_cb,
	.nvram		= &kiev_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sensor		= &kiev_sensor_cb,
	.sys 		= &kiev_sys_cb,
	.vpd		= &kiev_vpd_cb,
};

struct platform_intf platform_kiev = {
	.type		= PLATFORM_X86_64,
	.name		= "Kiev",
	.id_list	= kiev_id_list,
	.sub		= kiev_sub,
	.cb		= &kiev_cb,
	.probe		= &kiev_probe,
	.setup_post	= &kiev_setup_post,
	.destroy	= &kiev_destroy,
};
