/*
 * Copyright 2015, Google Inc.
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

#include "cyclone.h"

/* Treat each module as a logical "DIMM" */
#define CYCLONE_DIMM_COUNT	1

enum cyclone_memory_config {
	SAMSUNG_DDR3_1600_512M,
	NANYA_DDR3L_1600_512M,
	MEM_UNKNOWN,
};

enum cyclone_board_id {
	BOARD_ID_GALE_PROTO = 0, /* 000 */
	BOARD_ID_GALE_EVT = 1, /* 001 */
	BOARD_ID_GALE_EVT2_0 = 2, /* 010 */
	BOARD_ID_GALE_EVT2_1 = 6, /* 110 */
	BOARD_ID_GALE_EVT3 = 5, /* 101 */
	BOARD_ID_GALE_DVT_CONFIG_A = 7, /* 111 */
	BOARD_ID_GALE_DVT_CONFIG_B = 8, /* 002 */
	BOARD_ID_GALE_CR_CONFIG_A = 9, /* 012 */
	BOARD_ID_GALE_CR_CONFIG_B = 10, /* 020 */
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
	return CYCLONE_DIMM_COUNT;
}

static enum cyclone_memory_config get_memory_config(struct platform_intf *intf)
{
	uint32_t board_id;

	if (fdt_get_board_id(&board_id) >= 0) {
		switch (board_id) {
		case BOARD_ID_GALE_PROTO:
		case BOARD_ID_GALE_EVT:
		case BOARD_ID_GALE_EVT2_0:
		case BOARD_ID_GALE_EVT2_1:
		case BOARD_ID_GALE_EVT3:
		case BOARD_ID_GALE_DVT_CONFIG_A:
		case BOARD_ID_GALE_CR_CONFIG_A:
			return SAMSUNG_DDR3_1600_512M;
		case BOARD_ID_GALE_DVT_CONFIG_B:
		case BOARD_ID_GALE_CR_CONFIG_B:
			return NANYA_DDR3L_1600_512M;
		}
	}
	else {
		lprintf(LOG_DEBUG, "Unable to obtain board ID\n");
	}
	/* Return default memory type for Gale */
	if (!strcmp(intf->name, "Gale")) {
		return SAMSUNG_DDR3_1600_512M;
	}
	lprintf(LOG_ERR, "Unable to determine memory configuration\n");
	return MEM_UNKNOWN;
}

static int get_mem_info(struct platform_intf *intf,
			const struct nonspd_mem_info **info)
{
	switch (get_memory_config(intf)) {
	case SAMSUNG_DDR3_1600_512M:
		*info = &samsung_k4b4g1646e;
		break;
	case NANYA_DDR3L_1600_512M:
		*info = &nanya_ddr3l_nt5cc256m16dp_di;
		break;
	default:
		return -1;
	}

	return 0;
}

struct memory_cb cyclone_memory_cb = {
	.dimm_count		= dimm_count,
	.nonspd_mem_info	= &get_mem_info,
};
