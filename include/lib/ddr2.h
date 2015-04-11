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
 * Library-ized SPD info gathering routines for DDR2 SDRAM
 */

#ifndef LIB_DDR2_H__
#define LIB_DDR2_H__

enum ddr2_reg_map {
	/* 0 */
	DDR2_SPD_REG_NUM_BYTES_WRITTEN,
	DDR2_SPD_REG_TOTAL_BYTES,
	DDR2_SPD_REG_MEMORY_TYPE,
	DDR2_SPD_REG_NUM_ROWS,
	DDR2_SPD_REG_NUM_COLS,
	DDR2_SPD_REG_ATTRIBUTES_HEIGHT_PACKAGE_COC_RANKS,
	DDR2_SPD_REG_NUM_DATA_WIDTH,
	/* byte 7 is reserved */
	DDR2_SPD_REG_VOLTAGE_INTERFACE = 8,
	DDR2_SPD_REG_CYCLE_TIME_AT_MAX_CAS,
	/* 10 */
	DDR2_SPD_REG_ACCESS_FROM_CLOCK,
	DDR2_SPD_REG_DIMM_CONFIGURATION_TYPE,
	DDR2_SPD_REG_REFRESH_RATE,
	DDR2_SPD_REG_PRIMARY_SDRAM_WIDTH,
	DDR2_SPD_REG_ERROR_CHECKING_SDRAM_WIDTH,
	/* byte 15 reserved */
	DDR2_SPD_REG_ATTRIBUTES_BURST_LENGTH = 16,
	DDR2_SPD_REG_ATTRIBUTES_NUM_BANKS = 17,
	DDR2_SPD_REG_ATTRIBUTES_SUPPORTED_CAS_LATENCIES,
	DDR2_SPD_REG_MODULE_THICKNESS,
	DDR2_SPD_REG_MODULE_TYPE,
	/* 30 */
	/* 40 */
	/* 50 */
	/* 60 */
	DDR2_SPD_REG_CHECKSUM = 63,
	/* 70 */
	DDR2_SPD_REG_MFG_LOC = 72,
	DDR2_SPD_REG_MFG_PART_NUMBER_START,
	/* 90 */
	DDR2_SPD_REG_MFG_PART_NUMBER_END = 90,
	DDR2_SPD_REG_REVISION_CODE_START,
	DDR2_SPD_REG_REVISION_CODE_END = 92,
	DDR2_SPD_REG_SERIAL_NUMBER_START = 95,
	DDR2_SPD_REG_SERIAL_NUMBER_END = 98,
	/* 100 */
	/* 110 */
	/* 120 */
};

#endif /* LIB_DDR2_H__ */
