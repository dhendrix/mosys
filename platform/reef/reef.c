/*
 * Copyright 2016, Google Inc.
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
#include "lib/sku.h"
#include "lib/smbios.h"
#include "lib/elog.h"

#include "reef.h"

/* sku_info: brand, model, chassis, customization */
static struct sku_info
	/* Reef SKUs */
	/* TODO(hungte) Reef firmware updater was using 'reef' as model name
	 * for Electro and Basking. We should remove the 'reef' below when
	 * updater configuration is corrected.
	 */
	SKU_ELECTRO = { .brand = "ACBB", .model = "reef", .chassis = "ELECTRO"},
	SKU_BASKING = { .brand = "ASUN", .model = "reef", .chassis = "BASKING"},
	/* Coral SKUs */
	/* TODO(hungte) Fill the brand code when they are assigned. */
	SKU_ASTRONAUT = { .brand = NULL, .model = "astronaut", },
	SKU_SANTA = { .brand = NULL, .model = "santa", },
	SKU_LAVA = { .brand = NULL, .model = "lava", },
	SKU_PORBEAGLE = { .brand = NULL, .model = "porbeagle", },
	SKU_ROBO = { .brand = NULL, .model = "robo", };

/* Reference: b/35583395 */
static struct sku_mapping reef_sku_table[] = {
	{0, &SKU_BASKING},
	{8, &SKU_ELECTRO},
	{SKU_NUMBER_ANY, NULL},
};

/**
 * TODO(hungte) The mapping table below is made by duplicating for ranges. We do
 * this for now because it's unclear if most projects will be ranged, or is
 * sufficient with single or discontinuous values. We can revise this to ranges,
 * or other data structure in future when we have better idea of how the mapping
 * table looks like in the end.
 */
/* Reference: Coral SKU Map */
static struct sku_mapping coral_sku_table[] = {
	{0, &SKU_ASTRONAUT},
	{1, &SKU_ASTRONAUT},
	{2, &SKU_SANTA},
	{3, &SKU_SANTA},
	{4, &SKU_LAVA},
	{5, &SKU_LAVA},
	{26, &SKU_PORBEAGLE},
	{27, &SKU_PORBEAGLE},
	{70, &SKU_ROBO},
	{71, &SKU_ROBO},
	{SKU_NUMBER_ANY, NULL},
};

struct probe_ids {
	const char *names[2];
	/**
	 * Devices with SKU-based mapping should define sku_table,
	 * otherwise use single_sku.
	 */
	struct sku_mapping *sku_table;
	const struct sku_info single_sku;
};

static const struct probe_ids probe_id_list[] = {
	{ { "Reef", }, .sku_table = reef_sku_table },
	{ { "Coral", }, .sku_table = coral_sku_table },

	{ { "Pyro", }, .single_sku = { .brand = "LEAN" } },
	{ { "Sand", }, },
	{ { "Snappy", }, .single_sku = { .brand = "HPZO" } },
	{ { NULL }, },
};

struct platform_cmd *reef_sub[] = {
	&cmd_ec,
	&cmd_eeprom,
	&cmd_memory,
	&cmd_nvram,
	&cmd_pd,
	&cmd_platform,
	&cmd_smbios,
	&cmd_eventlog,
	NULL
};

int reef_probe(struct platform_intf *intf)
{
	static int status, probed;
	const struct probe_ids *pid;

	if (probed)
		return status;

	for (pid = probe_id_list; pid && pid->names[0]; pid++) {
		/* FRID */
		if (probe_frid((const char **)pid->names)) {
			status = 1;
			goto reef_probe_exit;
		}

		/* SMBIOS */
		if (probe_smbios(intf, (const char **)pid->names)) {
			status = 1;
			goto reef_probe_exit;
		}
	}
	return 0;

reef_probe_exit:
	probed = 1;
	/* Update canonical platform name */
	intf->name = pid->names[0];
	if (pid->sku_table) {
		intf->sku_info = sku_find_info(intf, pid->sku_table);
	} else {
		intf->sku_info = &pid->single_sku;
	}
	return status;
}

/* late setup routine; not critical to core functionality */
static int reef_setup_post(struct platform_intf *intf)
{
	if (cros_ec_setup(intf) < 0)
		return -1;
	return 0;
}

static int reef_destroy(struct platform_intf *intf)
{
	return 0;
}

struct eventlog_cb reef_eventlog_cb = {
	.print_type	= &elog_print_type,
	.print_data	= &elog_print_data,
	.print_multi	= &elog_print_multi,
	.verify		= &elog_verify,
	.verify_header	= &elog_verify_header,
	.fetch		= &elog_fetch_from_smbios,
};

struct platform_cb reef_cb = {
	.ec		= &cros_ec_cb,
	.eeprom		= &reef_eeprom_cb,
	.memory		= &reef_memory_cb,
	.nvram		= &reef_nvram_cb,
	.smbios		= &smbios_sysinfo_cb,
	.sys 		= &reef_sys_cb,
	.eventlog	= &reef_eventlog_cb,
};

struct platform_intf platform_reef = {
	.type		= PLATFORM_X86_64,
	.name		= "Reef",
	.sub		= reef_sub,
	.cb		= &reef_cb,
	.probe		= &reef_probe,
	.setup_post	= &reef_setup_post,
	.destroy	= &reef_destroy,
};
