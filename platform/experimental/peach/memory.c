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
#include "lib/math.h"
#include "lib/spd.h"

#include "peach.h"

/* Treat the chips on each channel as a logical DIMM */
#define PEACH_PIT_DIMM_COUNT	2

/* FIXME: double-check that these are final values... */
static const uint8_t samsung_ddr3_1600_1rank_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x12,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x03,
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 8 banks * 512Mbits = 4Gb */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 ranks, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x02,	/* 32-bit channel */

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
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_count(struct platform_intf *intf)
{
	return PEACH_PIT_DIMM_COUNT;
}


static int spd_read(struct platform_intf *intf,
			  int dimm, int reg, int len, uint8_t *buf)
{
	int rc = 0;

	switch (peach_board_config) {
	case PEACH_PIT_CONFIG_REV_0_0:
	case PEACH_PIT_CONFIG_REV_3_0:
	case PEACH_PIT_CONFIG_REV_4_0:
	case PEACH_PIT_CONFIG_REV_5_0:
	case PEACH_PIT_CONFIG_REV_6_0:
	case PEACH_PIT_CONFIG_REV_7_0:
	case PEACH_PIT_CONFIG_REV_9_0:
	case PEACH_PIT_CONFIG_REV_A_0:
	case PEACH_PIT_CONFIG_REV_B_0:
	case PEACH_PIT_CONFIG_REV_C_0:
	case PEACH_PIT_CONFIG_REV_D_0:
	case PEACH_PIT_CONFIG_REV_E_0:
	case PEACH_PI_CONFIG_REV_8_4:
	case PEACH_PI_CONFIG_REV_9_4:
		memcpy(buf, &samsung_ddr3_1600_1rank_spd[reg], len);
		rc = len;
		break;
	case PEACH_PIT_CONFIG_REV_7_2:
	case PEACH_PIT_CONFIG_REV_9_2:
	case PEACH_PIT_CONFIG_REV_A_2:
	case PEACH_PIT_CONFIG_REV_B_2:
	case PEACH_PIT_CONFIG_REV_C_2:
	case PEACH_PIT_CONFIG_REV_D_2:
	case PEACH_PIT_CONFIG_REV_E_2:
	case PEACH_PI_CONFIG_REV_A_6:
	case PEACH_PI_CONFIG_REV_B_6:
	case PEACH_PI_CONFIG_REV_C_6:
	case PEACH_PI_CONFIG_REV_D_6:
	case PEACH_PI_CONFIG_REV_E_6:
		/* 4GB models have the second rank populated */
		memcpy(buf, &samsung_ddr3_1600_1rank_spd[reg], len);
		buf[DDR3_SPD_REG_MODULE_ORG] &= ~__mask(5, 3);
		buf[DDR3_SPD_REG_MODULE_ORG] |= 1 << 3;
		rc = len;
		break;
	default:
		rc = -1;
		break;
	}

	return rc;
}

static struct memory_spd_cb spd_cb = {
	.read		= spd_read,
};

struct memory_cb peach_memory_cb = {
	.dimm_count	= dimm_count,
	.spd		= &spd_cb,
};
