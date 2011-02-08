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
 *
 * DDR3 field access for DDR3 SPDs.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mosys/platform.h"
#include "mosys/kv_pair.h"

#include "lib/string.h"
#include "lib/valstr.h"

#include "lib/ddr3.h"
#include "lib/spd.h"

#include "jedec_id.h"

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
	case SPD_GET_MFG_ID:
	{
		uint8_t manuf_lsb;
		uint8_t manuf_msb;
		const char *tstr;

		manuf_lsb = byte[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] & 0x7f;
		manuf_msb = byte[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB];

		tstr = jedec_manufacturer(manuf_lsb, manuf_msb);

		if (tstr != NULL) {
			kv_pair_fmt(kv, "mfg_id", "%u-%u: %s", manuf_lsb + 1,
			            manuf_msb, tstr);
		} else {
			kv_pair_fmt(kv, "mfg_id", "%u-%u", manuf_lsb + 1,
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
		manuf_msb = byte[DDR3_SPD_REG_DRAM_MANUF_JEDEC_ID_MSB];

		tstr = jedec_manufacturer(manuf_lsb, manuf_msb);

		if (tstr != NULL) {
			kv_pair_fmt(kv, "mfg_id_dram", "%u-%u: %s",
			            manuf_lsb + 1, manuf_msb, tstr);
		} else {
			kv_pair_fmt(kv, "mfg_id_dram", "%u-%u",
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
		kv_pair_fmt(kv, "serial_number", "0x%02x%02x%02x%02x",
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
		long size;

		/* calculate the total size in MB */
		size = 256 << (byte[DDR3_SPD_REG_DENSITY_BANKS] & 0xf);
		size >>= 3; /* in terms of bytes instead of bits. */
		size *= 8 << (byte[DDR3_SPD_REG_MODULE_BUS_WIDTH] & 0x7);
		size /= 4 << (byte[DDR3_SPD_REG_MODULE_ORG] & 0x7);
		size *= 1 + ((byte[DDR3_SPD_REG_MODULE_ORG] >> 3) & 0x7);

		kv_pair_fmt(kv, "size", "%llu", size);
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
		int i;
		int mhz;
		int one_added;
		char speeds[128];
		const struct valstr possible_mhz[] = {
			{ 400, "DDR3-800" },
			{ 533, "DDR3-1066" },
			{ 667, "DDR3-1333" },
			{ 800, "DDR3-1600" },
			{ 0 }
		};

		mhz = 1000 * byte[DDR3_SPD_REG_MTB_DIVISOR];
		mhz /= byte[DDR3_SPD_REG_MTB_DIVIDEND];
		mhz /= byte[DDR3_SPD_REG_TCK_MIN];

		memset(speeds, 0, sizeof(speeds));
		one_added = 0;
		for (i = 0; possible_mhz[i].val != 0; i++) {
			if (possible_mhz[i].val <= mhz) {
				if (one_added) {
					strcat(speeds, ", ");
				}
				one_added = 1;
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
