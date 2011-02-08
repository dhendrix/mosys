/*
 * Copyright (C) 2011 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/output.h"
#include "mosys/platform.h"

#include "lib/spd.h"

static int memory_spd_dump_cmd(struct platform_intf *intf,
			       struct platform_cmd *cmd, int argc, char **argv)
{
	struct spd_device *spd;
	uint8_t dimm;

	/* get dimm number from command line */
	if (argc < 1) {
		platform_cmd_usage(cmd);
		return -1;
	}
	dimm = (uint8_t) strtoul(argv[0], NULL, 0);

	lprintf(LOG_DEBUG, "memory spd dump %u\n", dimm);

	spd = new_spd_device(intf, dimm);
	if (spd == NULL) {
		lprintf(LOG_ERR, "Failed to read from SPD %u (not present?)\n",
				 dimm);
		return -1;
	}

	print_buffer(&spd->eeprom.data[0], spd->eeprom.length);

	free(spd);
	return 0;
}

#if 0
static int memory_spd_info_cmd(struct platform_intf *intf,
			       struct platform_cmd *cmd, int argc, char **argv)
{
	struct kv_pair *kv;
	struct spd_device *spd;
	uint8_t dimm;
	int i;

	/* get dimm number from command line */
	if (argc < 1) {
		platform_cmd_usage(cmd);
		return -1;
	}
	dimm = (uint8_t) strtoul(argv[0], NULL, 0);

	lprintf(LOG_DEBUG, "memory spd info %u\n", dimm);

	spd = new_spd_device(intf, dimm);
	if (spd == NULL) {
		lprintf(LOG_DEBUG,
			"Failed to read from SPD %u (not present?)\n", dimm);
		return -1;
	}

	/* print them all */
	for (i = 0; i < spd->eeprom.length; i++) {
		kv = kv_pair_new();
		kv_pair_fmt(kv, "dimm", "%u", dimm);
		if (spd_print_reg(intf, kv, &spd->eeprom.data[0], i) > 0)
			kv_pair_print(kv);
		kv_pair_free(kv);
	}

	free(spd);
	return 0;
}
#endif

#if 0
static int memory_spd_list(struct platform_intf *intf, int dimm)
{
	struct kv_pair *kv;
	struct spd_device *spd;
	uint8_t *spd_data;

	lprintf(LOG_DEBUG, "memory spd list %u\n", dimm);

	/* start line */
	kv = kv_pair_new();

	spd = new_spd_device(intf, dimm);
	if (spd == NULL) {
		lprintf(LOG_DEBUG,
			"Failed to read from SPD %u (not present?)\n", dimm);
		return 0;	/* not an error */
	}

	spd_data = &spd->eeprom.data[0];
	/* add various fields */
	kv_pair_fmt(kv, "dimm", "%u", dimm);
	spd_print_field(intf, kv, spd_data, SPD_GET_SPEEDS);
	spd_print_field(intf, kv, spd_data, SPD_GET_SIZE);
	spd_print_field(intf, kv, spd_data, SPD_GET_ECC);
	spd_print_field(intf, kv, spd_data, SPD_GET_RANKS);
	spd_print_field(intf, kv, spd_data, SPD_GET_WIDTH);
	spd_print_field(intf, kv, spd_data, SPD_GET_CHECKSUM);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_ID);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_ID_DRAM);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_LOC);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_DATE);
	spd_print_field(intf, kv, spd_data, SPD_GET_SERIAL_NUMBER);
	spd_print_field(intf, kv, spd_data, SPD_GET_PART_NUMBER);
	spd_print_field(intf, kv, spd_data, SPD_GET_REVISION_CODE);

	/* print raw spd if we are not in value-only print mode */
	if (mosys_get_kv_pair_style() != KV_STYLE_VALUE) {
		if (spd_print_raw(kv, spd->eeprom.length, spd_data) < 0)
			lprintf(LOG_DEBUG, "Unable to add raw SPD\n");
	}

	kv_pair_print(kv);
	kv_pair_free(kv);

	free(spd);
	return 0;
}

static int memory_spd_list_cmd(struct platform_intf *intf,
			       struct platform_cmd *cmd, int argc, char **argv)
{
	int ret = 0;
	uint8_t dimm;

	/* get dimm number from command line */
	if (argc < 1) {
		/* get total number of dimms */
		int total = 0;
		if (intf->cb->memory && intf->cb->memory->dimm_count)
			total = intf->cb->memory->dimm_count(intf);
		/* print list info for each one */
		for (dimm = 0; dimm < total; dimm++)
			ret |= memory_spd_list(intf, dimm);
	} else {
		/* print info for just one dimm */
		dimm = (uint8_t) strtoul(argv[0], NULL, 0);
		ret = memory_spd_list(intf, dimm);
	}

	return ret;
}
#endif

static int memory_spd_print_id(struct platform_intf *intf, int dimm)
{
	struct kv_pair *kv;
	struct spd_device *spd;
	uint8_t *spd_data;

	kv = kv_pair_new();

	spd = new_spd_device(intf, dimm);
	if (spd == NULL) {
		lprintf(LOG_DEBUG,
			"Failed to read from SPD %u (not present?)\n", dimm);
		return 0;	/* not an error */
	}

	spd_data = &spd->eeprom.data[0];
	/* add various fields */
	kv_pair_fmt(kv, "dimm", "%u", dimm);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_ID);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_ID_DRAM);
#if 0
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_LOC);
	spd_print_field(intf, kv, spd_data, SPD_GET_MFG_DATE);
#endif
	spd_print_field(intf, kv, spd_data, SPD_GET_SERIAL_NUMBER);
	spd_print_field(intf, kv, spd_data, SPD_GET_PART_NUMBER);

#if 0
	/* print raw spd if we are not in value-only print mode */
	if (mosys_get_kv_pair_style() != KV_STYLE_VALUE) {
		if (spd_print_raw(kv, spd->eeprom.length, spd_data) < 0)
			lprintf(LOG_DEBUG, "Unable to add raw SPD\n");
	}
#endif

	kv_pair_print(kv);
	kv_pair_free(kv);

	free(spd);
	return 0;
}

static int memory_spd_print_id_cmd(struct platform_intf *intf,
                                   struct platform_cmd *cmd,
                                   int argc, char **argv)
{
	int dimm = 0, last_dimm;

	if (!intf->cb->memory ||
	    !intf->cb->memory->dimm_count ||
	    !intf->cb->memory->dimm_spd)
		return -ENOSYS;

	if (argc) {
		dimm = (uint8_t)strtol(argv[0], NULL, 0);
		if ((dimm < 0) || (dimm > intf->cb->memory->dimm_count(intf))) {
			lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
			return -EINVAL;
		}
		last_dimm = dimm;
	}

	do {
		memory_spd_print_id(intf, dimm);
		dimm++;
	} while (dimm < last_dimm);

	return 0;
}

static struct platform_cmd memory_spd_print_cmds[] = {
#if 0
	{
		.name	= "geometry",
		.desc	= "Print module geometry and capacity",
		.usage	= "<dimm number>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = memory_spd_print_geometry }
	},
#endif
	{
		.name	= "id",
		.desc	= "Print module ID information",
		.usage	= "<dimm number>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = memory_spd_print_id_cmd }
	},
#if 0
	{
		.name	= "timing",
		.desc	= "Print module timing capabilities",
		.usage	= "<dimm number>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = memory_spd_print_timing }
	},
#endif
	{ NULL }
};

struct platform_cmd memory_spd_cmds[] = {
	{
		.name	= "print",
		.desc	= "Print SPD Information",
		.usage	= "[dimm number]",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = memory_spd_print_cmds }
	},
	{
		.name	= "dump",
		.desc	= "Dump SPD info as raw binary",
		.usage	= "<dimm number>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = memory_spd_dump_cmd }
	},
	{ NULL }
};