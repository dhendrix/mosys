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

#include <stdlib.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "drivers/google/cros_ec.h"

#include "lib/probe.h"
#include "lib/smbios.h"
#include "lib/elog.h"

#include "slippy.h"

struct probe_ids {
	const char *names[2];
	const char *frids[2];
};

static const struct probe_ids probe_id_list[] = {
	{ { "Falco", NULL },
	  { "Google_Falco", NULL },
	},
	{ { "Leon", NULL },
	  { "Google_Leon", NULL },
	},
	{ { "Peppy", NULL },
	  { "Google_Peppy", NULL },
	},
	{ { "Slippy", NULL },
	  { "Google_Slippy", NULL },
	},
	{ { "Wolf", NULL },
	  { "Google_Wolf", NULL },
	},
	{ { NULL } }
};

struct platform_cmd *slippy_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_smbios,
	&cmd_eventlog,
	NULL
};

int slippy_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const struct probe_ids *pid;

	if (probed)
		return status;

	for (pid = probe_id_list; pid && pid->names[0]; pid++) {
		/* FRID */
		if (probe_frid((const char **)pid->frids)) {
			status = 1;
			goto slippy_probe_exit;
		}

		/* SMBIOS */
		if (probe_smbios(intf, (const char **)pid->names)) {
			status = 1;
			goto slippy_probe_exit;
		}
	}
	return 0;

slippy_probe_exit:
	probed = 1;
	/* Update canonical platform name */
	intf->name = pid->names[0];
	return status;
}

/* late setup routine; not critical to core functionality */
static int slippy_setup_post(struct platform_intf *intf)
{
	int rc = 0;

	rc |= slippy_ec_setup(intf);
	if (rc)
		lprintf(LOG_DEBUG, "%s: failed\n", __func__);
	return rc;
}

static int slippy_destroy(struct platform_intf *intf)
{
	return 0;
}

struct eventlog_cb slippy_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
	.fetch		= &elog_fetch_from_smbios,
};

struct platform_cb slippy_cb = {
	.ec		= &cros_ec_cb,
	.eeprom		= &slippy_eeprom_cb,
	.gpio		= &slippy_gpio_cb,
	.memory		= &slippy_memory_cb,
	.nvram		= &slippy_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &slippy_sys_cb,
	.eventlog	= &slippy_eventlog_cb,
};

struct platform_intf platform_slippy = {
	.type		= PLATFORM_X86_64,
	.name		= "Slippy",
	.sub		= slippy_sub,
	.cb		= &slippy_cb,
	.probe		= &slippy_probe,
	.setup_post	= &slippy_setup_post,
	.destroy	= &slippy_destroy,
};
