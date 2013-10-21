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
 * This file contains functions and tables for dumping the various classes
 * of DIMM SPD we support, such as DDR, DDR2, FB-DIMM.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <valstr.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/math.h"
#include "lib/string.h"

#include "lib/spd.h"

#include "jedec_id.h"

/* Register Specific Functions */
const char *spd_table_lookup(struct spd_reg *reg,
                             const uint8_t * eeprom, uint8_t byte)
{
	static char buf[20];
	if (reg->table[eeprom[byte]])
		return reg->table[eeprom[byte]];
	snprintf(buf, sizeof(buf), "UNKNOWN (0x%02x)", eeprom[byte]);
	return buf;
}

/* return an unsigned number string */
const char *spd_readbyte(struct spd_reg *reg,
                         const uint8_t * eeprom, uint8_t byte)
{
	static char buf[4];

	snprintf(buf, sizeof(buf), "%u", eeprom[byte]);
	return buf;
}

/* return a hex number string */
const char *spd_readhex(struct spd_reg *reg,
                        const uint8_t * eeprom, uint8_t byte)
{
	static char buf[5];

	snprintf(buf, sizeof(buf), "%#x", eeprom[byte]);
	return buf;
}

const char *spd_burst(struct spd_reg *reg, const uint8_t * eeprom, uint8_t byte)
{
	static char buf[100];
	static char fmt[4];
	char first = 1;
	int i;

	//FIXME: I'm not sure this is correct.  Audit.
	buf[0] = '\0';
	for (i = 0; i < 8; i++) {
		if (eeprom[byte] & (1 << i)) {
			sprintf(fmt, "%d", 1 << i);
			if (!first)
				strcat(buf, ", ");
			first = 0;
			strcat(buf, fmt);
		}
	}
	return buf;
}

const char *spd_decimal_access_time(struct spd_reg *reg,
                                    const uint8_t * eeprom, uint8_t byte)
{
	static char buf[100];

	if (eeprom[byte] == 0)
		return "Derated operation not allowed";
	sprintf(buf, "%d.%02d", eeprom[byte] / 100, eeprom[byte] % 100);
	return buf;
}

const char *spd_shift_access_time(struct spd_reg *reg,
                                  const uint8_t * eeprom, uint8_t byte)
{
	static char buf[3];
	sprintf(buf, "%d", eeprom[byte] >> 2);
	return buf;
}

const char *spd_module_bank_density(struct spd_reg *reg,
                                    const uint8_t * eeprom, uint8_t byte)
{
	/*
	 * Module bank density starts at 0x08 meaning 32MB (0x10 is 64MB).
	 * Anything less is a roll over of the bits (0x01 is 1024MB).
	 */
	static char buf[5];
	uint16_t tmp = eeprom[byte] | (((uint16_t) eeprom[byte]) << 8);
	tmp >>= 3;
	sprintf(buf, "%d", tmp << 5);
	return buf;
}

const char *spd_shift_by(struct spd_reg *reg,
                         const uint8_t * eeprom, uint8_t byte)
{
	static char buf[100];
	sprintf(buf, "%d", 1 << eeprom[byte]);
	return buf;
}

const char *spd_revision_code(struct spd_reg *reg,
                              const uint8_t * eeprom, uint8_t byte)
{
	static char buf[6];
	sprintf(buf, "%d.%d", eeprom[byte] >> 4, eeprom[byte] & 0x0f);
	return buf;
}

/*
 * spd_total_size  -  determine total bytes in spd from first few bytes
 *
 * @data:	spd data
 *
 * returns total size of SPD, may be less than max depending on type of module
 * returns <0 to indicate failure
 *
 */
int spd_total_size(uint8_t *data)
{
	int size;

	switch (data[2]) {
	case SPD_DRAM_TYPE_DDR:
	case SPD_DRAM_TYPE_DDR2:{
		if (data[1] == 0) {
			lprintf(LOG_DEBUG, "Undefined SPD size, "
					   "assuming %d bytes\n",
					   SPD_MAX_LENGTH);
			size = SPD_MAX_LENGTH;
		} else {
			size = 1 << data[1];
		}

		break;
	}
	case SPD_DRAM_TYPE_DDR3:
	case SPD_DRAM_TYPE_LPDDR3:
	case SPD_DRAM_TYPE_FBDIMM:{
		uint8_t tmp;

		tmp = ((data[0] & __mask(6, 4)) >> 4);
		if (tmp == 0x1) {
			size = 256;
		} else {
			lprintf(LOG_DEBUG, "Undefined SPD size, "
					   "assuming %d bytes\n",
					   SPD_MAX_LENGTH);
			size = SPD_MAX_LENGTH;
		}

		break;
	}
	default:
		lprintf(LOG_ERR, "SPD type %02x not supported\n", data[2]);
		return -1;
	}

	if (size > SPD_MAX_LENGTH) {
		lprintf(LOG_DEBUG, "SPD-defined size %d too large, using "
				   "default size (%d)\n", size, SPD_MAX_LENGTH);
		size = SPD_MAX_LENGTH;
	}

	return size;
}

/*
 * spd_print_reg  -  add register to given key=value pair
 *
 * @intf:       platform interface
 * @kv:         key=value pair
 * @data:       raw spd data
 * @reg:        register to print
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
int spd_print_reg(struct platform_intf *intf,
		  struct kv_pair *kv, const void *data, uint8_t reg)
{
	int i;
	const uint8_t *byte = data;
	static const struct spd_callbacks *callbacks[] = {
//		&ddr1_callbacks,
		&ddr2_callbacks,
//		&fbdimm_callbacks,
	};

	if (!intf || !kv || !data)
		return -1;

	for (i = 0; i < sizeof(callbacks)/sizeof(callbacks[0]); i++) {
		const struct spd_callbacks *callback = callbacks[i];
		struct spd_reg *regmap;

		/* Continue if not matching SPD type. */
		if (callback->dram_type != byte[2]) {
			continue;
		}

		/* Desired register does not fall into provide range. */
		if (reg >= callback->num_regs) {
			return -1;
		}

		regmap = &callback->regs[reg];

		/* Register does not have a name. Return signalling that nothing
		 * was added to the kv_pair. */
		if (regmap->name == NULL) {
			return 0;
		}

		kv_pair_add(kv, "name", regmap->name);

		if (regmap->func)
			kv_pair_add(kv, "value",
			            regmap->func(regmap, data, reg));
		else
			kv_pair_fmt(kv, "value", "%d", byte[reg]);

		return 1;
	}

	lprintf(LOG_ERR, "SPD type %02x not supported\n", byte[2]);
	return -1;
}

/*
 * spd_print_raw - print raw SPD
 *
 * @kv:         key=value pair
 * @len:	number of spd bytes to print
 * @spd_data:	all spd data
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int spd_print_raw(struct kv_pair *kv, int len, uint8_t *spd_data)
{
	char *str;

	str = buf2str(spd_data, len);
	kv_pair_fmt(kv, "raw_spd", str);
	free(str);

	return 0;
}

/*
 * spd_print_field  -  add common SPD fields into key=value pair
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
int spd_print_field(struct platform_intf *intf,
		    struct kv_pair *kv,
		    const void *data, enum spd_field_type type)
{
	const uint8_t *byte = data;

	if (!intf || !kv || !data)
		return -1;

	switch (byte[2]) {
//	case SPD_DRAM_TYPE_DDR:
//		return spd_print_field_ddr1(intf, kv, data, type);
	case SPD_DRAM_TYPE_DDR2:
		return spd_print_field_ddr2(intf, kv, data, type);
//	case SPD_DRAM_TYPE_FBDIMM:
//		return spd_print_field_fbdimm(intf, kv, data, type);
	case SPD_DRAM_TYPE_DDR3:
	case SPD_DRAM_TYPE_LPDDR3:
		return spd_print_field_ddr3(intf, kv, data, type);
	default:
		lprintf(LOG_ERR, "SPD type %02x not supported\n", byte[2]);
	}

	return -1;
}
