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
 */

#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <valstr.h>

#include "mosys/alloc.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"

#include "lib/math.h"
#include "lib/string.h"

#include "lib/ddr2.h"
#include "lib/spd.h"

#include "jedec_id.h"

/* booleans to represent DIMM configuration type fields (byte 11) */
struct ddr2_config_type {
	uint8_t data_parity;
	uint8_t data_ecc;
	uint8_t address_command_parity;
};

static const struct valstr ddr2_module_type_lut[] = {
	{ 0x00, "Undefined" },
	{ 0x01, "RDIMM" },
	{ 0x02, "UDIMM" },
	{ 0x04, "SO-DIMM" },
	{ 0x06, "72b-SO-CDIMM" },
	{ 0x07, "72b-SO-RDIMM" },
	{ 0x08, "MICRO-DIMM" },
	{ 0x0a, "MINI-RDIMM" },
	{ 0x40, "MINI-UDIMM" },
};

static uint8_t ddr2_num_rows(const uint8_t *spd)
{
	/* bits 0-4 represent # row addresses, bits 5-7 are reserved,
	 * value of 0 means "undefined" */
	return spd[DDR2_SPD_REG_NUM_ROWS] & __mask(4, 0);
}

static uint8_t ddr2_num_cols(const uint8_t *spd)
{
	/* bits 0-3 represent # column addresses, bits 4-7 are reserved,
	 * value of 0 means "undefined" */
	return spd[DDR2_SPD_REG_NUM_COLS] & __mask(3, 0);
}

static uint8_t ddr2_num_ranks(const uint8_t *spd)
{
	return (spd[DDR2_SPD_REG_ATTRIBUTES_HEIGHT_PACKAGE_COC_RANKS]
	        & 0x07) + 1;	/* valid range is 0-7, offset is +1 */
}

/* returns width in bytes */
static uint8_t ddr2_data_width(const uint8_t *spd)
{
	/* entire range of uint8_t valid, 0 means "undefined" */
	return spd[DDR2_SPD_REG_NUM_DATA_WIDTH];
}

/* fills a dimm config structure, returns <0 to indicate failure */
static int ddr2_get_config_type(const uint8_t *spd,
			struct ddr2_config_type *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->data_parity = spd[DDR2_SPD_REG_DIMM_CONFIGURATION_TYPE]
	                   & (1 << 0) ? 1 : 0;
	cfg->data_ecc = spd[DDR2_SPD_REG_DIMM_CONFIGURATION_TYPE]
	                & (1 << 1) ? 1 : 0;
	cfg->address_command_parity = spd[DDR2_SPD_REG_DIMM_CONFIGURATION_TYPE]
	                              & (1 << 2) ? 1 : 0;

	return 0;
}

/* returns width in bytes */
static uint8_t ddr2_primary_data_width(const uint8_t *spd)
{
	/* entire range of uint8_t valid, 0 means "undefined" */
	return spd[DDR2_SPD_REG_PRIMARY_SDRAM_WIDTH];
}

static uint8_t ddr2_num_banks(const uint8_t *spd)
{
	/* entire range of uint8_t valid, 0 means "undefined" */
	return spd[DDR2_SPD_REG_ATTRIBUTES_NUM_BANKS];
}

static uint8_t ddr2_jedec_checksum(const uint8_t *spd)
{
	return spd[DDR2_SPD_REG_CHECKSUM];
}

static uint8_t ddr2_mfg_loc(const uint8_t *spd)
{
	return spd[DDR2_SPD_REG_MFG_LOC];
}

/* sets pointer to first byte of part number and returns the length */
static size_t ddr2_part_number(const uint8_t *spd, const uint8_t **part_num)
{

	*part_num = &spd[DDR2_SPD_REG_MFG_PART_NUMBER_START];
	return DDR2_SPD_REG_MFG_PART_NUMBER_END -
	       DDR2_SPD_REG_MFG_PART_NUMBER_START + 1;
}

/* calculates module size in bytes; 0 means "undefined" */
static uint64_t ddr2_module_size(const uint8_t *spd)
{
	uint8_t width, rows, cols, banks, ranks;

	/*
	 * calculate the total size in bytes:
	 * primary data width * 2^(#rows) * 2^(#cols) * #banks * #ranks
	 */
	width = ddr2_primary_data_width(spd);
	rows = ddr2_num_rows(spd);
	cols = ddr2_num_cols(spd);
	ranks = ddr2_num_ranks(spd);
	banks = ddr2_num_banks(spd);

	return width * (1ULL << rows) * (1ULL << cols) * banks * ranks;
}

/*
 * spd_print_field_ddr2  -  add common DDR SPD fields into key=value pair
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
int spd_print_field_ddr2(struct platform_intf *intf, struct kv_pair *kv,
                         const void *data, enum spd_field_type type)
{
	int ret = 0;
	const uint8_t *byte = data;

	switch (type) {
	case SPD_GET_DRAM_TYPE:
		kv_pair_add(kv, "dram", "DDR2");
		ret = 1;
		break;
	case SPD_GET_MODULE_TYPE:
		kv_pair_add(kv, "module",
		            val2str(byte[DDR2_SPD_REG_MODULE_TYPE],
		            ddr2_module_type_lut));
		ret = 1;
		break;
	case SPD_GET_MFG_ID:{ /* module manufacturer id */
		int table;
		const char *tstr;

		/* Bytes 64-71 specify a manufacturer's ID as a byte prefixed by
		 * n-1 continuation codes (0x7F). For example, Nanya's code is
		 * 0x7F7F7F0B, specifying bank 4, entry 11 in the Jedec's
		 * manufacturer ID list (JEP106). */
		for (table = 0; table < 7; table++) {
			if (byte[64 + table] != 0x7F)
				break;
		}
		tstr = jedec_manufacturer(table, (byte[64 + table]) & 0x7F);
		if (tstr) {
			kv_pair_fmt(kv, "module_mfg", "%u-%u: %s",
				    table + 1, byte[64 + table], tstr);
			ret = 1;
		} else {
			kv_pair_fmt(kv, "module_id", "%u-%u",
				    table + 1, byte[64 + table]);
			ret = 1;
		}
		break;
	}

	case SPD_GET_MFG_LOC: /* manufacturing loction */
		kv_pair_fmt(kv, "mfg_loc", "0x%02x",
		            ddr2_mfg_loc(byte));
		ret = 1;
		break;

	case SPD_GET_MFG_DATE: /* manufacturing date (BCD values) */
		kv_pair_fmt(kv, "mfg_date", "20%02x-wk%02x", byte[93],
			    byte[94]);
		ret = 1;
		break;

	case SPD_GET_SERIAL_NUMBER: /* module serial number */
		kv_pair_fmt(kv, "serial_number", "%02x%02x%02x%02x",
			    byte[95], byte[96], byte[97], byte[98]);
		ret = 1;
		break;

	case SPD_GET_PART_NUMBER:{ /* module part number */
		const uint8_t *part_num = NULL;
		size_t len;

		len = ddr2_part_number(byte, &part_num);
		if (part_num == NULL) {
			ret = 0;
			break;
		}

		/* The raw part number is not ASCII null-terminated, so we must
		 * set the precision modifier in lieu of a terminator. */
		kv_pair_fmt(kv, "part_number", "%.*s", len, part_num);

		ret = 1;
		break;
	}

	case SPD_GET_REVISION_CODE: /* module revision code */
		kv_pair_fmt(kv, "revision_code", "0x%02x%02x", byte[91],
			    byte[92]);
		ret = 1;
		break;

	case SPD_GET_SIZE:
		/* get size (in bytes) and convert to MB (2^20) */
		kv_pair_fmt(kv, "size_mb", "%llu",
		            ddr2_module_size(byte) >> 20);
		ret = 1;
		break;

	case SPD_GET_ECC:{ /* ECC capability */
		struct ddr2_config_type cfg;

		if (ddr2_get_config_type(byte, &cfg) < 0) {
			ret = 0;
			break;
		}
		kv_pair_add_bool(kv, "ecc", cfg.data_ecc);
		ret = 1;
		break;
	}

	case SPD_GET_RANKS: /* number of ranks */
		kv_pair_fmt(kv, "ranks", "%u", ddr2_num_ranks(byte));
		ret = 1;
		break;

	case SPD_GET_WIDTH: /* DRAM device width */
		kv_pair_fmt(kv, "width", "%u", ddr2_data_width(byte));
		ret = 1;
		break;

	case SPD_GET_CHECKSUM: /* SPD checksum */
		kv_pair_fmt(kv, "checksum", "0x%02x",
		            ddr2_jedec_checksum(byte));
		ret = 1;
		break;

	case SPD_GET_SPEEDS:{ /* module frequency/CAS capabilities */
		char speeds[128]; 	/* String holding all valid speeds */
		int i;
		uint8_t cas_supported;
		int max_cas;
		const char *clock;
		const char *cas;
		struct valstr ddr2_cas_lut[] = {
			{ 0x10, "TBD" },	/* TBD */
			{ 0x15, "TBD" },	/* TBD */
			{ 0x20, "2" },
			{ 0x30, "3" },
			{ 0x40, "4" },
			{ 0x50, "5" },
			{ 0x60, "6" },
			{ 0x70, "7" },
		};
		struct valstr ddr2_clock_lut[] = {
			/* { tCKmin, DDR2-designation } */
			{ 0xA0, "DDR2-1600" }, 	/* 200MHz */
			{ 0x75, "DDR2-2100" }, 	/* 266MHz */
			{ 0x60, "DDR2-2700" },	/* 333MHz */
			{ 0x50, "DDR2-3200" }, 	/* 400MHz */
			{ 0x46, "DDR2-3500" }, 	/* 433MHz */
			{ 0x42, "DDR2-3700" }, 	/* 466MHz */
			{ 0x3d, "DDR2-4200" }, 	/* 533MHz */
			{ 0x30, "DDR2-5300" }, 	/* 667MHz */
			{ 0x25, "DDR2-6400" }, 	/* 800MHz */
			{ 0x20, "DDR2-8000" }, 	/* 1000MHz */
		};

		cas_supported = byte[18] & 0xFC;

		for (i = 7; i >= 0; i--) {
			if (cas_supported & (1 << i))
				break;
		}

		max_cas = i;
		/* Find the max cycle time / CL pairing */
		cas = ddr2_cas_lut[i].str;
		clock = val2str(byte[9], ddr2_clock_lut);
		snprintf(speeds, sizeof(speeds), "%s-%s", clock, cas);

		/* Find the derated max cycle time at CLX - 1.0 */
		if (cas_supported & (1 << (max_cas - 1))) {
			char tmp[32] = { '\0' };
			cas = ddr2_cas_lut[i - 1].str;
			clock = val2str(byte[23], ddr2_clock_lut);

			snprintf(tmp, sizeof(speeds), " %s-%s", clock, cas);
			strncat(speeds, tmp, sizeof(speeds) - strlen(speeds) - 1);
		}

		/* Find the derated max cycle time at CLX - 2.0 */
		if (cas_supported & (1 << (max_cas - 2))) {
			char tmp[32] = { '\0' };
			cas = ddr2_cas_lut[i - 2].str;
			clock = val2str(byte[25], ddr2_clock_lut);

			snprintf(tmp, sizeof(speeds), " %s-%s", clock, cas);
			strncat(speeds, tmp, sizeof(speeds) - strlen(speeds) - 1);
		}

		kv_pair_add(kv, "speeds", speeds);
		ret = 1;
		break;
	}
	default:
		return 0;
	}

	return ret;
}
