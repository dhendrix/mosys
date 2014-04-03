/*
 * Copyright 2014, Google Inc.
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

#include "intf/mmio.h"

#include "lib/ddr3.h"
#include "lib/spd.h"

#include "nyan.h"

/* Treat each module as a logical "DIMM" */
#define NYAN_DIMM_COUNT	4

static uint8_t hynix_ddr3_1866_256x16_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x12,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x00,	/* undefined */
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 8 banks, 4Gb */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 rank, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x01,	/* 16 data lines per module */

	/* DDR3-1866 = (1/8)ns * 9 + (-54 * 0.001ns) = 1.071ns */
	[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR]	= 0x11,	/* granularity of 1ps */
	[DDR3_SPD_REG_MTB_DIVIDEND]		= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]		= 8,
	[DDR3_SPD_REG_TCK_MIN]			= 9,

	/* 6, 7, 8, 9, 10, 11, 13 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0xfc,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x02,

	/* CL-tRCD-tRP at 1866: 13-13-13 */
	[DDR3_SPD_REG_TAA_MIN]		= 0x6f,	/* 13.91ns (approx. 13.875ns)*/
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x6f,	/* 13.91ns (approx 13.875ns) */

	[DDR3_SPD_REG_FINE_OFFSET_TCK_MIN]	= 0xca,		/* -54 (2C) */

	/* SK Hynix is bank 1, 0x2d (=> 0xad with parity bit) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 0x80,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 0xad,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'H',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = '5',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = 'T',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = 'C',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = 'G',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = '3',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = 'A',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = 'F',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = 'R',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'R',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = 'D',
	[DDR3_SPD_REG_MODULE_PART_NUM_14] = '0',
	[DDR3_SPD_REG_MODULE_PART_NUM_15] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_16] = 0,
	[DDR3_SPD_REG_MODULE_PART_NUM_17] = 0,
};

static uint8_t hynix_ddr3_1600_256x16_spd[SPD_MAX_LENGTH] = {
	[DDR3_SPD_REG_SIZE_CRC]		= 0x92,
	[DDR3_SPD_REG_REVISION]		= 0x12,
	[DDR3_SPD_REG_DEVICE_TYPE]	= 0x0b,
	[DDR3_SPD_REG_MODULE_TYPE]	= 0x00,	/* undefined */
	[DDR3_SPD_REG_DENSITY_BANKS]	= 0x04,	/* 8 banks, 4Gb */
	[DDR3_SPD_REG_ADDRESSING]	= 0x32,	/* 15 rows, 10 cols */
	[DDR3_SPD_REG_VOLTAGE]		= 0x02,	/* 1.35V */
	[DDR3_SPD_REG_MODULE_ORG]	= 0x02,	/* 1 rank, x16 */
	[DDR3_SPD_REG_MODULE_BUS_WIDTH]	= 0x01,	/* 16 data lines per module */

	/* DDR3-1600 = (1/8)ns * 10 = 1.25ns */
	[DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR]	= 0x11,
	[DDR3_SPD_REG_MTB_DIVIDEND]		= 1,
	[DDR3_SPD_REG_MTB_DIVISOR]		= 8,
	[DDR3_SPD_REG_TCK_MIN]			= 9,

	/* 5, 6, 7, 8, 9, 10, 11 */
	[DDR3_SPD_REG_CAS_LAT_LSB]	= 0xfe,
	[DDR3_SPD_REG_CAS_LAT_MSB]	= 0x00,

	/* CL-tRCD-tRP at 1600: 11-11-11 */
	[DDR3_SPD_REG_TAA_MIN]		= 0x6e,	/* 13.75ns */
	[DDR3_SPD_REG_TWR_MIN]		= 0x78,	/* 15ns */
	[DDR3_SPD_REG_TRCD_MIN]		= 0x6e,	/* 13.75ns */

	/* SK Hynix is bank 1, 0x2d (=> 0xad with parity bit) */
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB] = 0x80,
	[DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB] = 0xad,

	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_0] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_1] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_2] = 0x00000000,
	[DDR3_SPD_REG_MODULE_MANUF_SERIAL_3] = 0x00000000,

	[DDR3_SPD_REG_MODULE_PART_NUM_0] = 'H',
	[DDR3_SPD_REG_MODULE_PART_NUM_1] = '5',
	[DDR3_SPD_REG_MODULE_PART_NUM_2] = 'T',
	[DDR3_SPD_REG_MODULE_PART_NUM_3] = 'C',
	[DDR3_SPD_REG_MODULE_PART_NUM_4] = '4',
	[DDR3_SPD_REG_MODULE_PART_NUM_5] = 'G',
	[DDR3_SPD_REG_MODULE_PART_NUM_6] = '6',
	[DDR3_SPD_REG_MODULE_PART_NUM_7] = '3',
	[DDR3_SPD_REG_MODULE_PART_NUM_8] = 'A',
	[DDR3_SPD_REG_MODULE_PART_NUM_9] = 'F',
	[DDR3_SPD_REG_MODULE_PART_NUM_10] = 'R',
	[DDR3_SPD_REG_MODULE_PART_NUM_11] = '-',
	[DDR3_SPD_REG_MODULE_PART_NUM_12] = 'P',
	[DDR3_SPD_REG_MODULE_PART_NUM_13] = 'B',
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
	return NYAN_DIMM_COUNT;
}

/* TODO: Move this stuff if more Tegra-specific drivers are added. */
#define TEGRA_APB_MISC_BASE			0x70000000
#define TEGRA_PMC_BASE				(TEGRA_APB_MISC_BASE + 0xe400)
#define TEGRA_APBDEV_PMC_STRAPPING_OPT_A	(TEGRA_PMC_BASE + 0x464)
static int get_ramcode(struct platform_intf *intf)
{
	uint32_t opts;
	static uint32_t ramcode;
	static int done;

	if (done)
		return ramcode;

	if (mmio_read32(intf, TEGRA_APBDEV_PMC_STRAPPING_OPT_A, &opts) < 0) {
		lprintf(LOG_ERR, "%s: Cannot read strapping opts\n", __func__);
		return -1;
	}

	ramcode = (opts >> 4) & 0xf;
	done = 1;
	return ramcode;
}

enum nyan_memory_config {
	HYNIX_DDR3_1600_2G,
	HYNIX_DDR3_1600_4G,
	HYNIX_DDR3_1866_2G,
	HYNIX_DDR3_1866_4G,
	MEM_UNKNOWN,
};

static enum nyan_memory_config get_memory_config(struct platform_intf *intf)
{
	int ramcode;
	enum nyan_memory_config memory_config = MEM_UNKNOWN;

	ramcode = get_ramcode(intf);
	if (ramcode < 0)
		return -1;

	switch (get_nyan_type(intf)) {
	case NYAN:
		if (ramcode == 0x0)
			memory_config = HYNIX_DDR3_1866_2G;
		else if (ramcode == 0x1)
			memory_config = HYNIX_DDR3_1600_4G;
		break;
	case NYAN_BIG:
	case NYAN_BLAZE:
		if (ramcode == 0x1)
			memory_config = HYNIX_DDR3_1600_2G;
		else if (ramcode == 0x4)
			memory_config = HYNIX_DDR3_1600_4G;
		break;
	default:
		memory_config = MEM_UNKNOWN;
		break;
	}

	return memory_config;
}

static int spd_read(struct platform_intf *intf,
			  int dimm, int reg, int len, uint8_t *buf)
{
	uint8_t *p;
	int system_has_4gbytes = 0;

	switch (get_memory_config(intf)) {
	case HYNIX_DDR3_1600_4G:
		system_has_4gbytes = 1;
	case HYNIX_DDR3_1600_2G:
		p = hynix_ddr3_1600_256x16_spd;
		break;
	case HYNIX_DDR3_1866_4G:
		system_has_4gbytes = 1;
	case HYNIX_DDR3_1866_2G:
		p = hynix_ddr3_1866_256x16_spd;
		break;
	default:
		return -1;
	}

	/* We assume homogeneous modules on a given system, so no need to
	 * revert the value for 2GB systems. */
	if (system_has_4gbytes) {
		p[DDR3_SPD_REG_DENSITY_BANKS]	= 0x05;	/* 8 banks, 8Gb */
		p[DDR3_SPD_REG_MODULE_PART_NUM_4] = '8';
	}

	memcpy(buf, &p[reg], len);
	return len;
}

static struct memory_spd_cb spd_cb = {
	.read		= spd_read,
};

struct memory_cb nyan_memory_cb = {
	.dimm_count	= dimm_count,
	.spd		= &spd_cb,
};
