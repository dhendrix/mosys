/*
 * Copyright 2013, Google Inc.
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

#include "skate.h"

/* Each channel will be represented as a logical DIMM */
#define SKATE_DIMM_COUNT	2

/* Fake SPD data for now (based off Elpida SPD for Snow) */
static const uint8_t fake_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x10,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 8 banks * 512Mbits = 4Gb */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 rank, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x02,	/* 32-bits */

	/* DDR3-1666 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_MTB_DIVIDEND]	= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]	= 8,
	[DDR3_SPD_REG_TCK_MIN]		= 10,

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

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'F',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = 'A',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = 'K',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = 'E',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = ' ',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = 'S',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = 'P',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = 'D',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = '\0',
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

/* SPD data for Micron modules meeting Skate's spec */
static const uint8_t micron_ddr3_1600_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x12,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 8 banks * 512Mbits = 4Gb */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 rank, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x02,	/* 32-bits */

	/* DDR3-1600 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_MTB_DIVIDEND]	= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]	= 8,
	[DDR3_SPD_REG_TCK_MIN]		= 10,

	/* 6, 11 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0x84,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x00,

	/* CL-tRCD-tRP: 11-11-11 */
	[DDR3_SPD_REG_TAA_MIN]		= 0x6e,	/* 13.75ns */
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x6e,	/* 13.75ns */

	/* Micron is bank 1, number 44 (JEP-106) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 0,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 44,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'M',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = 'T',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = '1',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = 'K',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = '2',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '5',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = 'M',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = '1',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = 'H',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'A',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = '1',
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = '2',
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = '5',
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 'E',
};

/* SPD data for Nanya modules meeting Skate's spec */
static const uint8_t nanya_ddr3_1600_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x12,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 8 banks * 512Mbits = 4Gb */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 rank, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x02,	/* 32-bits */

	/* DDR3-1600 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_MTB_DIVIDEND]	= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]	= 8,
	[DDR3_SPD_REG_TCK_MIN]		= 10,

	/* 5, 6, 7, 8, 9, 10, 11 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0xfc,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x00,

	/* CL-tRCD-tRP: 11-11-11 */
	[DDR3_SPD_REG_TAA_MIN]		= 0x6e,	/* 13.75ns */
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x6e,	/* 13.75ns */

	/* Nanya is bank 4, number 11 (JEP-106) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 3,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 11,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'N',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = 'T',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = '5',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = 'C',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = 'C',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = '2',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '5',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = 'M',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = '1',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = 'B',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'P',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = 'D',
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = 'I',
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

/*
 * skate_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int skate_dimm_count(struct platform_intf *intf)
{
	return SKATE_DIMM_COUNT;
}


static int skate_spd_read(struct platform_intf *intf,
			  int dimm, int reg, int len, uint8_t *buf)
{
	int rc = 0;

	switch (skate_board_config) {
	case SKATE_CONFIG_PROTO_MICRON:
	case SKATE_CONFIG_EVT_MICRON:
	case SKATE_CONFIG_DVT_MICRON:
	case SKATE_CONFIG_PVT_MICRON:
	case SKATE_CONFIG_MP_MICRON:
		memcpy(buf, &micron_ddr3_1600_spd[reg], len);
		rc = len;
		break;
	case SKATE_CONFIG_PROTO_HYNIX:
	case SKATE_CONFIG_EVT_HYNIX:
	case SKATE_CONFIG_DVT_HYNIX:
	case SKATE_CONFIG_PVT_HYNIX:
	case SKATE_CONFIG_MP_HYNIX:
		/* FIXME: need to add correct timings */
		memcpy(buf, &fake_spd[reg], len);
		rc = len;
	case SKATE_CONFIG_PROTO_ELPIDA:
	case SKATE_CONFIG_EVT_ELPIDA:
	case SKATE_CONFIG_DVT_ELPIDA:
	case SKATE_CONFIG_PVT_ELPIDA:
	case SKATE_CONFIG_MP_ELPIDA:
		/* FIXME: need to add correct timings */
		memcpy(buf, &fake_spd[reg], len);
		rc = len;
		break;
	default:
		rc = -1;
		break;
	}

	return rc;
}

static struct memory_spd_cb skate_spd_cb = {
	.read		= skate_spd_read,
};

struct memory_cb skate_memory_cb = {
	.dimm_count	= skate_dimm_count,
	.spd		= &skate_spd_cb,
};
