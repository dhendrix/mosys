/*
 * Copyright 2012, Google Inc.
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
 * SPD-like memory info for memory without real SPD data.
 */

#ifndef LIB_NONSPD_H__
#define LIB_NONSPD_H__

#include <inttypes.h>

#include "lib/spd.h"

struct nonspd_mem_info {
	/* DRAM type */
	enum spd_dram_type dram_type;

	/* DIMM type */
	union {
		enum ddr3_module_type ddr3_type;
	} module_type;

	/* Module Size (in mbits) */
	unsigned int module_size_mbits;

	/* Number of ranks */
	unsigned int num_ranks;

	/* SDRAM device width (including ECC) */
	unsigned int device_width;

	/* module frequency capabilities (allow multiple entries for various
	 * de-rated frequency values) */
	enum ddr_freq ddr_freq[10];

	/* Module Manufacturer ID */
	struct {
		uint8_t lsb;
		uint8_t msb;
	} module_mfg_id;

	/* Module Manufacturing Location */
	uint8_t mfg_loc;

	/* Module Manufacturing Date */
	struct {
		uint8_t year;
		uint8_t week;
	} module_mfg_date;

	/* Module Serial Number */
	uint8_t serial_num[4];

	/* Module Part Number */
	uint8_t part_num[19];

	/* DRAM Manufacturer ID */
	struct {
		uint8_t lsb;
		uint8_t msb;
	} dram_mfg_id;

	/* Module revision code */
	uint8_t revision[2];
};

extern int nonspd_print_field(struct kv_pair *kv,
				const struct nonspd_mem_info *info,
				enum spd_field_type type);

/*
 * Modules defined in lib/spd/nonspd_modules.c
 */
extern const struct nonspd_mem_info elpida_8gbit_lpddr3_edfa164a2ma_jd_f;
extern const struct nonspd_mem_info elpida_8gbit_lpddr3_f8132a3ma_gd_f;
extern const struct nonspd_mem_info elpida_16gbit_lpddr3_fa232a2ma_gc_f;
extern const struct nonspd_mem_info hynix_4gbit_ddr3l_h5tc4g63afr_pba;
extern const struct nonspd_mem_info hynix_4gbit_ddr3l_h5tc4g63cfr_pba;
extern const struct nonspd_mem_info hynix_2gbit_lpddr3_h9ccnnn8gtmlar_nud;
extern const struct nonspd_mem_info hynix_4gbit_lpddr3_h9ccnnnbjtmlar_nud;
extern const struct nonspd_mem_info hynix_8gbit_ddr3l_h5tc8g63amr_pba;
extern const struct nonspd_mem_info hynix_8gbit_lpddr3_h9ccnnn8gtmlar_nud;
extern const struct nonspd_mem_info micron_mt41k256m16ha;
extern const struct nonspd_mem_info nanya_ddr3l_nt5cc256m16dp_di;
extern const struct nonspd_mem_info samsung_k4b4g1646d;
extern const struct nonspd_mem_info samsung_k4b4g1646e;
extern const struct nonspd_mem_info samsung_4gbit_ddr3l_k4b4g1646d_byk0;
extern const struct nonspd_mem_info samsung_4gbit_ddr3l_k4b4g1646q_hyk0;
extern const struct nonspd_mem_info samsung_8gbit_ddr3l_k4b8g1646q_myk0;
extern const struct nonspd_mem_info samsung_2gbit_lpddr3_k3qf2f20em_agce;
extern const struct nonspd_mem_info samsung_4gbit_lpddr3_k4e6e304ee_egce;
extern const struct nonspd_mem_info samsung_8gbit_lpddr3_k4e8e304ed_egcc;
extern const struct nonspd_mem_info samsung_2gbit_lpddr3_k4e8e304ee_egce;

#endif /* LIB_NONSPD_H__ */
