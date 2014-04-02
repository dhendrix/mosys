/* Copyright 2012, Google Inc.
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
 * DDR3 field access for DDR3 SPDs.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <valstr.h>

#include "mosys/platform.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"

#include "lib/string.h"

#include "lib/ddr3.h"
#include "lib/spd.h"

#include "jedec_id.h"


static const struct valstr ddr3_module_type_lut[] = {
	{ 0x00, "Undefined" },
	{ 0x01, "RDIMM" },
	{ 0x02, "UDIMM" },
	{ 0x03, "SO-DIMM" },
	{ 0x04, "MICRO-DIMM" },
	{ 0x05, "MINI-RDIMM" },
	{ 0x06, "MINI-UDIMM" },
	{ 0x07, "MINI-CDIMM" },
	{ 0x08, "72b-SO-UDIMM" },
	{ 0x09, "72b-SO-RDIMM" },
	{ 0x0a, "72b-SO-CDIMM" },
	{ 0x0b, "LRDIMM" },
};

/*
 * spd_print_field_ddr3  -  add common DDR SPD fields into key=value pair
 *
 * @intf:       platform interface
 * @kv:         key=value pair
 * @data:       raw spd data
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
int spd_print_field_ddr3(struct platform_intf *intf, struct kv_pair *kv,
                         const void *data, enum spd_field_type type)
{
	int ret;
	const uint8_t *byte = data;

	ret =  0;
	switch (type) {
	case SPD_GET_DRAM_TYPE:
		kv_pair_add(kv, "dram", "DDR3");
		ret = 1;
		break;
	case SPD_GET_MODULE_TYPE:
		kv_pair_add(kv, "module",
		            val2str(byte[DDR3_SPD_REG_MODULE_TYPE],
		            ddr3_module_type_lut));
		ret = 1;
		break;
	case SPD_GET_MFG_ID:
	{
		uint8_t manuf_lsb;
		uint8_t manuf_msb;
		const char *tstr;

		manuf_lsb = byte[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] & 0x7f;
		manuf_msb = byte[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] & 0x7f;

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
		uint8_t manuf_lsb;
		uint8_t manuf_msb;
		const char *tstr;

		manuf_lsb = byte[DDR3_SPD_REG_DRAM_MANUF_JEDEC_ID_LSB] & 0x7f;
		manuf_msb = byte[DDR3_SPD_REG_DRAM_MANUF_JEDEC_ID_MSB] & 0x7f;

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
		kv_pair_fmt(kv, "mfg_loc", "0x%02x",
		            byte[DDR3_SPD_REG_MODULE_MANUF_LOC]);
		ret = 1;
		break;
	}

	case SPD_GET_MFG_DATE: /* manufacturing date (BCD values) */
	{
		uint8_t year;
		uint8_t week;

		year = byte[DDR3_SPD_REG_MODULE_MANUF_DATE_YEAR];
		week = byte[DDR3_SPD_REG_MODULE_MANUF_DATE_YEAR];
		kv_pair_fmt(kv, "mfg_date", "20%02x-wk%02x", week, year);
		ret = 1;
		break;
	}

	case SPD_GET_SERIAL_NUMBER:
	{
		kv_pair_fmt(kv, "serial_number", "%02x%02x%02x%02x",
		            byte[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0],
		            byte[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1],
		            byte[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2],
		            byte[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3]);
		ret = 1;
		break;
	}

	case SPD_GET_PART_NUMBER:
	{
		char part[19];

		memcpy(part, &byte[DDR3_SPD_REG_MODULE_PART_NUM_0], 18);
		part[18] = '\0';
		kv_pair_fmt(kv, "part_number", "%s", part);

		ret = 1;
		break;
	}

	case SPD_GET_REVISION_CODE:
	{
		kv_pair_fmt(kv, "revision_code", "0x%02x%02x",
		            byte[DDR3_SPD_REG_MODULE_REVISION_0],
		            byte[DDR3_SPD_REG_MODULE_REVISION_1]);
		ret = 1;
		break;
	}

	case SPD_GET_SIZE:
	{
		/* See "Calculating Module Capacity" section in DDR3 SPD
		 * specification for details. */
		unsigned int size;

		/* calculate the total size in MB */
		size = 256 << (byte[DDR3_SPD_REG_DENSITY_BANKS] & 0xf);
		size >>= 3; /* in terms of bytes instead of bits. */
		size *= 8 << (byte[DDR3_SPD_REG_MODULE_BUS_WIDTH] & 0x7);
		size /= 4 << (byte[DDR3_SPD_REG_MODULE_ORG] & 0x7);
		size *= 1 + ((byte[DDR3_SPD_REG_MODULE_ORG] >> 3) & 0x7);

		kv_pair_fmt(kv, "size_mb", "%u", size);
		ret = 1;
		break;
	}

	case SPD_GET_ECC:
	{
		uint8_t bus_ext_width = byte[DDR3_SPD_REG_MODULE_BUS_WIDTH];
		bus_ext_width >>= 3;
		bus_ext_width &= 0x7;
		kv_pair_add_bool(kv, "ecc", bus_ext_width);
		ret = 1;
		break;
	}

	case SPD_GET_RANKS:
	{
		kv_pair_fmt(kv, "ranks", "%d",
		            1 + ((byte[DDR3_SPD_REG_MODULE_ORG] >> 3) & 0x7));
		ret = 1;
		break;
	}

	case SPD_GET_WIDTH:
	{
		/* Total width including ECC. */
		uint8_t width;
		width = 8 << (byte[DDR3_SPD_REG_MODULE_BUS_WIDTH] & 0x7);
		width += 8 * ((byte[DDR3_SPD_REG_MODULE_BUS_WIDTH] >> 3) & 0x7);
		kv_pair_fmt(kv, "width", "%d", width);
		ret = 1;
		break;
	}

	case SPD_GET_CHECKSUM:
	{
		kv_pair_fmt(kv, "checksum", "0x%02x%02x",
		            byte[DDR3_SPD_REG_CRC_1],
		            byte[DDR3_SPD_REG_CRC_0]);
		ret = 1;
		break;
	}

	case SPD_GET_SPEEDS:
	{
		int i, mhz, first_entry;
		char speeds[128];
		const struct valstr possible_mhz[] = {
			{ 400,  "DDR3-800" },
			{ 533,  "DDR3-1066" },
			{ 667,  "DDR3-1333" },
			{ 800,  "DDR3-1600" },
			{ 933,  "DDR3-1866" },
			{ 1067, "DDR3-2133" },
			{ 0 }
		};
		int tck_mtb = byte[DDR3_SPD_REG_TCK_MIN];
		int mtb_dividend = byte[DDR3_SPD_REG_MTB_DIVIDEND];
		int mtb_divisor = byte[DDR3_SPD_REG_MTB_DIVISOR];
		int ftb_dividend = byte[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR] >> 4;
		int ftb_divisor = byte[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR] & 0xf;
		double tck_ns, mtb = 0.0, ftb_ns = 0.0;
		/* fine offset is encoded in 2's complement format */
		int8_t ftb_offset = byte[DDR3_SPD_REG_FINE_OFFSET_TCK_MIN];

		/* Sanity check that MTB and FTB values are >=1 (as per spec) */
		ret = -1;
		if (!mtb_dividend)
			lprintf(LOG_ERR, "Invalid MTB dividend from SPD\n");
		else if (!mtb_divisor)
			lprintf(LOG_ERR, "Invalid MTB divisor from SPD\n");
		else if (!ftb_dividend)
			lprintf(LOG_ERR, "Invalid FTB dividend from SPD\n");
		else if (!ftb_divisor)
			lprintf(LOG_ERR, "Invalid FTB divisor from SPD\n");
		else
			ret = 0;
		if (ret)
			break;

		mtb = (double)mtb_dividend / mtb_divisor;
		ftb_ns = ((double)(ftb_dividend) / ftb_divisor) / 1000;
		tck_ns = tck_mtb * mtb + (ftb_offset * ftb_ns);
		mhz = (int)((double)1000/tck_ns);

		lprintf(LOG_DEBUG, "%s: %d * %.03fns + %d * %.03fns = %.02fns,"
				" mhz = %d\n", __func__,
				tck_mtb, mtb, ftb_offset, ftb_ns, tck_ns, mhz);

		memset(speeds, 0, sizeof(speeds));
		first_entry = 1;
		for (i = 0; possible_mhz[i].val != 0; i++) {
			double min = possible_mhz[i].val * 0.99;

			if (min <= mhz) {
				if (!first_entry)
					strcat(speeds, ", ");
				first_entry = 0;
				strcat(speeds, possible_mhz[i].str);
			}
		}

		kv_pair_add(kv, "speeds", speeds);
		ret = 1;
		break;
	}

	default:
	{
		ret = 0;	/* force "we don't handle this here */
		break;
	}
	}

	return ret;
}
