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
 */

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/ddr3.h"
#include "lib/spd.h"

#include "daisy.h"

#define DAISY_DIMM_COUNT	1

/* SPD data for Elpida (DDR3L-1600K) */
const uint8_t elpida_ddr3_1600_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x10,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x03,	/* 8 banks * 256MB = 2GBytes */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x01,	/* 1 rank, x8 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x03,

	/* DDR3-1666 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR]	= 0x11,	/* granularity of 1ps */
	[DDR3_SPD_REG_MTB_DIVIDEND]		= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]		= 8,
	[DDR3_SPD_REG_TCK_MIN]			= 10,

	/* 6, 7, 8, 9, 10, 11 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0xfc,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x00,

	[DDR3_SPD_REG_TAA_MIN]		= 0x69,	/* 13.125ns */
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x69,	/* 13.125ns */

	/* Elpida is bank 3, number 126 (JEP-106) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 2,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 126,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'E',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = 'D',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = 'J',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = '2',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = '1',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = '0',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '8',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = 'E',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = 'D',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = 'B',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = 'G',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'G',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = 'N',
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = 'F',
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = '\0',
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

/* SPD data for Samsung (DDR3L-1600K) */
const uint8_t samsung_ddr3_1600_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x10,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x03,	/* 8 banks * 256MB = 2GBytes */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x03,	/* 1.35V and 1.5V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x01,	/* 1 ranks, x8 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x03,

	/* DDR3-1666 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR]	= 0x11,	/* granularity of 1ps */
	[DDR3_SPD_REG_MTB_DIVIDEND]	= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]	= 8,
	[DDR3_SPD_REG_TCK_MIN]		= 10,

	/* 5, 6, 7, 8, 9, 10, 11 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0xfe,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x00,

	[DDR3_SPD_REG_TAA_MIN]		= 0x6e,	/* 13.75ns */
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x6e,	/* 13.75ns */

	/* Samsung is bank 1, number 78 (JEP-106) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 0,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 78,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'K',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = 'B',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = '2',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = 'G',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = '0',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '8',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = 'D',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = 'H',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'Y',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = 'K',
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = '0',
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = '\0',
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

/* SPD data for Samsung (DDR3Lx16-1600K) */
const uint8_t samsung_ddr3x16_1600_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x11,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 4Gb(512MB) * 4 banks = 2GBytes  */
	[DDR3_SPD_REG_ADDRESSING]	= 0x19,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V and 1.5V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 ranks, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x03,

	/* DDR3-1666 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR]	= 0x11,	/* granularity of 1ps */
	[DDR3_SPD_REG_MTB_DIVIDEND]		= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]		= 8,
	[DDR3_SPD_REG_TCK_MIN]			= 10,

	/* 5, 6, 7, 8, 9, 10, 11 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0xfe,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x00,

	[DDR3_SPD_REG_TAA_MIN]		= 0x69,	/* 13.125ns */
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x68,	/* 13.125ns */

	/* Samsung is bank 1, number 78 (JEP-106) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 0x80,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 0xce,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'K',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = 'B',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = 'G',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = '1',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = 'B',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = 'H',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'Y',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = 'K',
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = '0',
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = '\0',
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

/*
 * daisy_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int daisy_dimm_count(struct platform_intf *intf)
{
	return DAISY_DIMM_COUNT;
}


static int daisy_spd_read(struct platform_intf *intf,
			  int dimm, int reg, int len, uint8_t *buf)
{
	int rc = 0;

	/*
	 * For now we only support SPD commands on Snow. We will need to
	 * update this for Daisy eventually.
	 */
	switch (board_config) {
	case SNOW_CONFIG_ELPIDA_EVT:
	case SNOW_CONFIG_ELPIDA_DVT:
	case SNOW_CONFIG_ELPIDA_PVT:
	case SNOW_CONFIG_ELPIDA_PVT2:
	case SNOW_CONFIG_ELPIDA_MP:
		memcpy(buf, &elpida_ddr3_1600_spd[reg], len);
		rc = len;
		break;
	case SNOW_CONFIG_SAMSUNG_EVT:
	case SNOW_CONFIG_SAMSUNG_DVT:
	case SNOW_CONFIG_SAMSUNG_PVT:
	case SNOW_CONFIG_SAMSUNG_PVT2:
	case SNOW_CONFIG_SAMSUNG_MP:
		memcpy(buf, &samsung_ddr3_1600_spd[reg], len);
		rc = len;
		break;
	case SNOW_CONFIG_SAMSUNG_MP_1_2:
	case SNOW_CONFIG_SAMSUNG_MP_2_0:
		memcpy(buf, &samsung_ddr3x16_1600_spd[reg], len);
		rc = len;
		break;
	default:
		rc = -1;
	}

	return rc;
}

static struct memory_spd_cb daisy_spd_cb = {
	.read		= daisy_spd_read,
};

struct memory_cb daisy_memory_cb = {
	.dimm_count	= daisy_dimm_count,
	.spd		= &daisy_spd_cb,
};
