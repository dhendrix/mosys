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

/* booleans to represent DIMM configuration type fields (byte 11) */
struct ddr2_config_type {
	uint8_t data_parity;
	uint8_t data_ecc;
	uint8_t address_command_parity;
};

/*
 * ddr2_num_rows - obtain number of row addresses for DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * returns number of rows as reported by SPD
 */
extern uint8_t ddr2_num_rows(const uint8_t *spd);

/*
 * ddr2_num_cols - obtain number of column addresses for DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * Output of 0 has the special meaning of being undefined.
 *
 * returns number of column addresses as reported by SPD
 */
extern uint8_t ddr2_num_cols(const uint8_t *spd);

/*
 * ddr2_num_ranks - obtain number of ranks for DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * Output of 0 has the special meaning of being undefined.
 *
 * returns number of ranks as reported by SPD
 */
extern uint8_t ddr2_num_ranks(const uint8_t *spd);

/*
 * ddr2_data_width - obtain data width of DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * Data width includes primary data + error checking.
 * Output of 0 has the special meaning of being undefined.
 *
 * returns module data width as reported by SPD
 */
extern uint8_t ddr2_data_width(const uint8_t *spd);

/*
 * ddr2_get_dimm_config - fills dimm configuration information
 *
 * @spd:	entire spd content
 * @cfg:	dimm configuration to fill out
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
extern int ddr2_get_config_type(const uint8_t *spd,
                                struct ddr2_config_type *cfg);

/*
 * ddr2_primary_data_width - obtain data width of DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * Reports primary data width only, excludes error checking byte(s).
 * Output of 0 has the special meaning of being undefined.
 *
 * returns module primary data width as reported by SPD
 */
extern uint8_t ddr2_primary_data_width(const uint8_t *spd);

/*
 * ddr2_num_banks - obtain number of banks installed onto DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * returns number of banks as reported by SPD
 */
extern uint8_t ddr2_num_banks(const uint8_t *spd);

/*
 * ddr2_jedec_checksum - obtain JEDEC checksum of DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * JEDEC checksum is the remainder of the dividend of the sum of byte values
 * for bytes 0 to 62
 *
 * returns checksum as reported by SPD
 */
extern uint8_t ddr2_jedec_checksum(const uint8_t *spd);

/*
 * ddr2_mfg_loc - obtain manufacturing location for DDR2 memory module
 *
 * @spd:	entire spd content
 *
 * Manufacturing location is vendor-defined.
 *
 * returns manufacturing location as reported by SPD
 */
extern uint8_t ddr2_mfg_loc(const uint8_t *spd);

/*
 * ddr2_serial_number - set user-supplied pointer to first byte of serial number
 *
 * @spd:	entire spd content
 * @serial:	pointer to set
 *
 * returns length of serial number
 */
extern size_t ddr2_serial_number(const uint8_t *spd, const uint8_t **serial);

/*
 * ddr2_part_number - set user-supplied pointer to first byte of part number
 *
 * @spd:	entire spd content
 * @serial:	pointer to set
 *
 * returns length of part number
 */
extern size_t ddr2_part_number(const uint8_t *spd, const uint8_t **part_num);

/*
 * ddr2_revision_code - set user-supplied pointer to first byte of revision code
 *
 * @spd:	entire spd content
 * @rev:	pointer to set
 *
 * returns length of revision code
 */
extern size_t ddr2_revision_code(const uint8_t *spd, const uint8_t **rev);

/*
 * ddr2_module_size - calculates module size from various parameters
 *
 * @spd:	entire spd content
 *
 * If the size is undefined (return value is 0), then the SPD probably needs
 * to be reprogrammed.
 *
 * returns total module size/capacity in bytes (0 means "undefined")
 */
extern uint64_t ddr2_module_size(const uint8_t *spd);

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
