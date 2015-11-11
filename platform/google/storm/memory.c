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

#include "lib/fdt.h"
#include "lib/nonspd.h"

#include "mosys/log.h"
#include "mosys/platform.h"

#include "storm.h"

/* Treat each module as a logical "DIMM" */
#define STORM_DIMM_COUNT	2

enum storm_memory_config {
	HYNIX_DDR3L_1600_1G,
	MICRON_DDR3L_1600_1G,
	NANYA_DDR3L_1600_1G,
	SAMSUNG_DDR3_1600_1G,
	MEM_UNKNOWN,
};

enum storm_board_id {
	BOARD_ID_PROTO_0 = 0,
	BOARD_ID_PROTO_0_2 = 1,
	BOARD_ID_WHIRLWIND = 2,
	BOARD_ID_WHIRLWIND_SP5 = 3,
	BOARD_ID_WHIRLWIND_NT5CC256M16DP_DI = 4,
	BOARD_ID_ARKHAM = 6,
	BOARD_ID_ARKHAM_H5TC4G63CFR_PBA = 7,
	BOARD_ID_PROTO_0_2_NAND = 26,
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
	/* same for whirlwind and Arkham */
	return STORM_DIMM_COUNT;
}

static enum storm_memory_config get_memory_config(struct platform_intf *intf)
{
	uint32_t board_id;

	if (fdt_get_board_id(&board_id) < 0) {
		lprintf(LOG_ERR, "Unable to obtain RAM code.\n");
		return MEM_UNKNOWN;
	}

	switch (board_id) {
	/* Arkham */
	case BOARD_ID_ARKHAM:
		return MICRON_DDR3L_1600_1G;
	case BOARD_ID_ARKHAM_H5TC4G63CFR_PBA:
		return HYNIX_DDR3L_1600_1G;
	/* Storm */
	case BOARD_ID_PROTO_0:
	case BOARD_ID_PROTO_0_2:
	case BOARD_ID_PROTO_0_2_NAND:
		return SAMSUNG_DDR3_1600_1G;
	/* Whirlwind */
	case BOARD_ID_WHIRLWIND:
	case BOARD_ID_WHIRLWIND_SP5:
		return MICRON_DDR3L_1600_1G;
	case BOARD_ID_WHIRLWIND_NT5CC256M16DP_DI:
		return NANYA_DDR3L_1600_1G;
	default:
		lprintf(LOG_ERR, "Unable to determine memory configuration\n");
	}

	return MEM_UNKNOWN;
}

static int get_mem_info(struct platform_intf *intf,
			const struct nonspd_mem_info **info)
{
	switch (get_memory_config(intf)) {
	case HYNIX_DDR3L_1600_1G:
		*info = &hynix_4gbit_ddr3l_h5tc4g63cfr_pba;
		break;
	case MICRON_DDR3L_1600_1G:
		*info = &micron_mt41k256m16ha;
		break;
	case NANYA_DDR3L_1600_1G:
		*info = &nanya_ddr3l_nt5cc256m16dp_di;
		break;
	case SAMSUNG_DDR3_1600_1G:
		*info = &samsung_k4b4g1646d;
		break;
	default:
		return -1;
	}

	return 0;
}

struct memory_cb storm_memory_cb = {
	.dimm_count		= dimm_count,
	.nonspd_mem_info	= &get_mem_info,
};
