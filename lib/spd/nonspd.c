/* Copyright 2014, Google Inc.
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
 *
 * nonspd.c: Functions for pretty printing memory info for systems without SPD.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <valstr.h>

#include "jedec_id.h"
#include "lib/math.h"
#include "lib/nonspd.h"
#include "lib/string.h"
#include "lib/string_builder.h"
#include "mosys/platform.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"

/*
 * nonspd_print_field  -  add common DDR SPD fields into key=value pair
 *
 * @kv:         key=value pair
 * @info:       nonspd memory info
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
int nonspd_print_field(struct kv_pair *kv,
			const struct nonspd_mem_info *info,
			enum spd_field_type type)
{
	int ret = 0;

	switch (type) {
	case SPD_GET_DRAM_TYPE:
		switch (info->dram_type) {
		case SPD_DRAM_TYPE_DDR:
			kv_pair_add(kv, "dram", "DDR3");
			break;
		case SPD_DRAM_TYPE_DDR2:
			kv_pair_add(kv, "dram", "DDR2");
			break;
		case SPD_DRAM_TYPE_FBDIMM:
			kv_pair_add(kv, "dram", "FBDIMM");
			break;
		case SPD_DRAM_TYPE_DDR3:
			kv_pair_add(kv, "dram", "DDR3");
			break;
		case SPD_DRAM_TYPE_LPDDR3:
			kv_pair_add(kv, "dram", "LPDDR3");
			break;
		case SPD_DRAM_TYPE_LPDDR4:
			kv_pair_add(kv, "dram", "LPDDR4");
			break;
		default:
			break;
		}
		ret = 1;
		break;

	case SPD_GET_MODULE_TYPE:
		switch (info->dram_type) {
		case SPD_DRAM_TYPE_DDR3:
		case SPD_DRAM_TYPE_LPDDR3:
			kv_pair_add(kv, "module",
			            val2str(info->module_type.ddr3_type,
					    ddr3_module_type_lut));
			ret = 1;
			break;
		case SPD_DRAM_TYPE_DDR:
		case SPD_DRAM_TYPE_DDR2:
		case SPD_DRAM_TYPE_FBDIMM:
		default:
			ret = -1;
			break;
		}
		break;

	case SPD_GET_MFG_ID:
	{
		uint8_t manuf_lsb = info->module_mfg_id.lsb & 0x7f;
		uint8_t manuf_msb = info->module_mfg_id.msb & 0x7f;
		const char *tstr;

		tstr = jedec_manufacturer(manuf_lsb, manuf_msb);

		if (tstr != NULL) {
			kv_pair_fmt(kv, "module_mfg", "%u-%u: %s", manuf_lsb + 1,
			            manuf_msb, tstr);
		} else {
			kv_pair_fmt(kv, "module_mfg", "%u-%u", manuf_lsb + 1,
			            manuf_msb);
		}
		ret = 1;
		break;
	}

	case SPD_GET_MFG_ID_DRAM:
	{
		uint8_t manuf_lsb = info->dram_mfg_id.lsb & 0x7f;
		uint8_t manuf_msb = info->dram_mfg_id.msb & 0x7f;
		const char *tstr;

		tstr = jedec_manufacturer(manuf_lsb, manuf_msb);

		if (tstr != NULL) {
			kv_pair_fmt(kv, "dram_mfg", "%u-%u: %s",
			            manuf_lsb + 1, manuf_msb, tstr);
		} else {
			kv_pair_fmt(kv, "dram_mfg", "%u-%u",
			            manuf_lsb + 1, manuf_msb);
		}
		ret = 1;
		break;
	}

	case SPD_GET_MFG_LOC:
	{
		kv_pair_fmt(kv, "mfg_loc", "0x%02x", info->mfg_loc);
		ret = 1;
		break;
	}

	case SPD_GET_MFG_DATE: /* manufacturing date (BCD values) */
	{
		uint8_t year = info->module_mfg_date.year;
		uint8_t week = info->module_mfg_date.week;

		kv_pair_fmt(kv, "mfg_date", "20%02x-wk%02x", week, year);
		ret = 1;
		break;
	}

	case SPD_GET_SERIAL_NUMBER:
	{
		kv_pair_fmt(kv, "serial_number", "%02x%02x%02x%02x",
		            info->serial_num[0], info->serial_num[1],
		            info->serial_num[2], info->serial_num[3]);
		ret = 1;
		break;
	}

	case SPD_GET_PART_NUMBER:
	{
		char part[sizeof(info->part_num)+1];

		memcpy(part, &info->part_num[0], sizeof(info->part_num));
		part[sizeof(info->part_num)] = '\0';
		kv_pair_fmt(kv, "part_number", "%s", part);

		ret = 1;
		break;
	}

	case SPD_GET_REVISION_CODE:
	{
		kv_pair_fmt(kv, "revision_code", "0x%02x%02x",
		            info->revision[0],
			    info->revision[1]);
		ret = 1;
		break;
	}

	case SPD_GET_SIZE:
	{
		/* translate mbits to mbytes */
		kv_pair_fmt(kv, "size_mb", "%u", info->module_size_mbits / 8);
		ret = 1;
		break;
	}

	case SPD_GET_RANKS:
	{
		kv_pair_fmt(kv, "ranks", "%d", info->num_ranks);
		ret = 1;
		break;
	}

	case SPD_GET_WIDTH:
	{
		kv_pair_fmt(kv, "width", "%d", info->device_width);
		ret = 1;
		break;
	}

	case SPD_GET_SPEEDS:
	{
		int i, first_entry = 1;
		struct string_builder *speeds = new_string_builder();

		for (i = 0; i < ARRAY_SIZE(info->ddr_freq); i++) {
			if (!info->ddr_freq[i])
				continue;

			if (!first_entry)
				string_builder_strcat(speeds, ", ");
			else
				first_entry = 0;

			switch (info->dram_type) {
			case SPD_DRAM_TYPE_DDR3:
				string_builder_strcat(speeds, "DDR3-");
				break;
			case SPD_DRAM_TYPE_LPDDR3:
				string_builder_strcat(speeds, "LPDDR3-");
				break;
			case SPD_DRAM_TYPE_LPDDR4:
				string_builder_strcat(speeds, "LPDDR4-");
				break;
			default:
				break;
			}

			string_builder_strcat(speeds,
				ddr_freq_prettyprint[info->ddr_freq[i]]);
		}

		kv_pair_add(kv, "speeds", string_builder_get_string(speeds));
		free_string_builder(speeds);
		ret = 1;
		break;
	}

	default:
		break;
	}

	return ret;
}
