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

#include "lib/generic_callbacks.h"
#include "lib/probe.h"
#include "lib/smbios.h"
#include "lib/elog.h"

#include "auron.h"

struct probe_ids {
	const char *names[2];
	const char *frids[2];
	const int has_dimms;
};

struct platform_cb auron_cb;

static const struct probe_ids probe_id_list[] = {
	{ { "Buddy", NULL },
	  { "Google_Buddy", NULL },
	  1
	},
	{ { "Cid", NULL },
	  { "Google_Cid", NULL },
	  0
	},
	{ { "Gandof", NULL },
	  { "Google_Gandof", NULL },
	  0
	},
	{ { "Lulu", NULL },
	  { "Google_Lulu", NULL },
	  0
	},
	{ { "Paine", NULL },
	  { "Google_Auron_Paine", NULL },
	  0
	},
	{ { "Yuna", NULL },
	  { "Google_Auron_Yuna", NULL },
	  0
	},
	/*
	 * Leave this entry last in the table -- otherwise it will break
	 * frid matching, since Google_Auron is a prefix of Google_Auron_VAR
	 */
	{ { "Auron", NULL },
	  { "Google_Auron", NULL },
	  0
	},
	{ { NULL } }
};

struct platform_cmd *auron_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_psu,
	&cmd_smbios,
	&cmd_eventlog,
	NULL
};

int auron_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const struct probe_ids *pid;

	if (probed)
		return status;

	for (pid = probe_id_list; pid && pid->names[0]; pid++) {

		/* FRID */
		if (probe_frid((const char **)pid->frids)) {
			status = 1;
			goto auron_probe_exit;
		}

		/* SMBIOS */
		if (probe_smbios(intf, (const char **)pid->names)) {
			status = 1;
			goto auron_probe_exit;
		}
	}
	return 0;

auron_probe_exit:
	probed = 1;
	/* Update canonical platform name */
	intf->name = pid->names[0];
	if (pid->has_dimms == 1)
		auron_cb.memory = &dimm_memory_cb;
	return status;
}

/* late setup routine; not critical to core functionality */
static int auron_setup_post(struct platform_intf *intf)
{
	return cros_ec_setup(intf);
}

static int auron_destroy(struct platform_intf *intf)
{
	return 0;
}

struct eventlog_cb auron_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
	.fetch		= &elog_fetch_from_smbios,
};

struct platform_cb auron_cb = {
	.ec		= &cros_ec_cb,
	.pd		= &cros_pd_cb,
	.eeprom		= &auron_eeprom_cb,
	.gpio		= &auron_gpio_cb,
	.memory		= &auron_memory_cb,
	.nvram		= &auron_nvram_cb,
	.psu		= &generic_psu_battery_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &auron_sys_cb,
	.eventlog	= &auron_eventlog_cb,
};

struct platform_intf platform_auron = {
	.type		= PLATFORM_X86_64,
	.name		= "Auron",
	.sub		= auron_sub,
	.cb		= &auron_cb,
	.probe		= &auron_probe,
	.setup_post	= &auron_setup_post,
	.destroy	= &auron_destroy,
};
