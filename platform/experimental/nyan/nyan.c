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

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "drivers/gpio.h"
#include "drivers/google/cros_ec.h"

#include "lib/file.h"
#include "lib/math.h"
#include "lib/probe.h"

#include "nyan.h"

const char *nyan_id_list[] = {
	"NVIDIA Tegra124 Venice2",
	NULL
};

struct platform_cmd *nyan_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_gpio,
	&cmd_memory,
	&cmd_nvram,
	&cmd_platform,
	&cmd_eventlog,
	NULL
};

/* TODO: replace this with proper FDT parsing */
#define FDT_MODEL_NODE	"/proc/device-tree/model"
static char *model_from_fdt(void)
{
	int fd;
	static char model[32];
	int len;

	fd = file_open(FDT_MODEL_NODE, FILE_READ);
	if (fd < 0) {
		lperror(LOG_DEBUG, "Unable to open %s", FDT_MODEL_NODE);
		return NULL;
	}

	memset(model, 0, sizeof(model));
	len = read(fd, &model, sizeof(model));
	if (len < 0) {
		lprintf(LOG_DEBUG, "%s: Could not read FDT\n", __func__);
		return NULL;
	}

	return model;
}

int nyan_probe(struct platform_intf *intf)
{
	const char **id;
	char *model = NULL;
	int found = 0;

	model = model_from_fdt();
	if (!model)
		return -1;

	for (id = nyan_id_list; id && *id; id++) {
		lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ", model, *id);
		/*
		 * Allow for partial match since FDT-provided model may
		 * include extra revision info at the end.
		 */
		if (!strncmp(*id, model, strlen(*id))) {
			lprintf(LOG_DEBUG, "yes\n");
			found = 1;
			break;
		} else {
			lprintf(LOG_DEBUG, "no\n");
		}
	}

	return found;
}


static int nyan_setup_post(struct platform_intf *intf)
{
	if (nyan_ec_setup(intf) <= 0)
		return -1;

	return 0;
}

static int nyan_destroy(struct platform_intf *intf)
{
	intf->cb->ec->destroy(intf);
	return 0;
}

struct eventlog_cb nyan_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
	.add		= &elog_add_event_manually,
	.clear		= &elog_clear_manually,
	.fetch		= &elog_fetch_from_flash,
	.write		= &elog_write_to_flash,
};

struct platform_cb nyan_cb = {
	.ec 		= &cros_ec_cb,
	.nvram		= &cros_ec_nvram_cb,
	.sys 		= &nyan_sys_cb,
	.eventlog	= &nyan_eventlog_cb,
};

struct platform_intf platform_nyan = {
	.type		= PLATFORM_ARMV7,
	.name		= "Nyan",
	.id_list	= nyan_id_list,
	.sub		= nyan_sub,
	.cb		= &nyan_cb,
	.probe		= &nyan_probe,
	.setup_post	= &nyan_setup_post,
	.destroy	= &nyan_destroy,
};
