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
#include "lib/elog.h"

#include "parrot.h"

static const char *probed_platform_id;

const char *parrot_id_list[] = {
	"Parrot",
	NULL
};

struct platform_cmd *parrot_sub[] = {
	&cmd_eeprom,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_vpd,
	&cmd_ec,
	&cmd_eventlog,
	NULL
};

static const char *hwids[] = {
	"X86 PARROT",
};

static const char *frids[] = {
	"Google_Parrot",
	NULL
};

int parrot_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;

	if (probed)
		return status;

	if (probe_hwid(hwids)) {
		status = 1;
		goto parrot_probe_exit;
	}

	if (probe_frid(frids)) {
		status = 1;
		goto parrot_probe_exit;
	}

	if (probe_smbios(intf, parrot_id_list)) {
		status = 1;
		goto parrot_probe_exit;
	}

parrot_probe_exit:
	probed = 1;
	return status;
}

/* late setup routine; not critical to core functionality */
static int parrot_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	/* FIXME: until VPD is properly implemented, do not fail on setup */
	if (parrot_vpd_setup(intf) < 0)
		lprintf(LOG_INFO, "VPD not found\n");

	rc |= parrot_eeprom_setup(intf);

	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int parrot_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);

	/* FIXME: unmap vpd stuff */
	return 0;
}

struct eventlog_cb parrot_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
};

struct platform_cb parrot_cb = {
	.eeprom		= &parrot_eeprom_cb,
	.memory		= &parrot_memory_cb,
	.nvram		= &parrot_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &parrot_sys_cb,
	.vpd		= &parrot_vpd_cb,
	.ec		= &parrot_ec_cb,
	.eventlog	= &parrot_eventlog_cb,
};

struct platform_intf platform_parrot = {
	.type		= PLATFORM_X86_64,
	.name		= "Parrot",
	.id_list	= parrot_id_list,
	.sub		= parrot_sub,
	.cb		= &parrot_cb,
	.probe		= &parrot_probe,
	.setup_post	= &parrot_setup_post,
	.destroy	= &parrot_destroy,
};
