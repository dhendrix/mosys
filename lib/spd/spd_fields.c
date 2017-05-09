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
	case SPD_DRAM_TYPE_DDR4:
	  size = 384;
	  break;
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
	case SPD_DRAM_TYPE_DDR4:
	  return spd_print_field_ddr4(intf, kv, data, type);
	  break;
	default:
		lprintf(LOG_ERR, "SPD type %02x not supported\n", byte[2]);
	}

	return -1;
}
