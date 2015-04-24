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

#include "oak.h"

#define OAK_DIMM_COUNT	1

const struct nonspd_mem_info hynix_8gbit_lpddr3_h9ccnnn8gtmlar_nud = {
	.dram_type              = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type  = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits      = 16384,
	.num_ranks              = 1,
	.device_width           = 32,
	.ddr_freq               = { DDR_533, DDR_667, DDR_800 },

	.module_mfg_id          = { .msb = 0xad, .lsb = 0x80 },
	.dram_mfg_id            = { .msb = 0xad, .lsb = 0x80 },

	.part_num =
		{ 'H', '9', 'C', 'C', 'N', 'N', 'N', '8', 'G', 'T', 'M', 'L',
		  'A', 'R', '-', 'N', 'U', 'D',},
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
	return OAK_DIMM_COUNT;

}

static int get_mem_info(struct platform_intf *intf,
			const struct nonspd_mem_info **info)
{
	*info = &hynix_8gbit_lpddr3_h9ccnnn8gtmlar_nud;
	return 0;
}

struct memory_cb oak_memory_cb = {
	.dimm_count		= dimm_count,
	.nonspd_mem_info	= &get_mem_info,
};
