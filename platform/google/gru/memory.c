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

#include "gru.h"

static int gru_dimm_count;
static const struct nonspd_mem_info *gru_mem_info;

/* read RAM code and fill in values needed by memory commands */
static int read_ram_code(struct platform_intf *intf)
{
	static int done = 0;
	static int ret = 0;
	uint32_t ram_code;

	if (done)
		return ret;

	if (fdt_get_ram_code(&ram_code) < 0) {
		lprintf(LOG_ERR, "Unable to obtain RAM code.\n");
		return -1;
	}

	if (!strncmp(intf->name, "Gru", 3)) {
		switch (ram_code) {
		case 0:
			gru_dimm_count = 2;
			gru_mem_info = &samsung_lpddr3_k4e6e304eb_egcf;
			break;
		default:
			ret = -1;
			break;
		}
	} else if (!strncmp(intf->name, "Kevin", 5)) {
		switch (ram_code) {
		case 0:
			gru_dimm_count = 2;
			gru_mem_info = &samsung_lpddr3_k4e6e304eb_egcf;
			break;
		default:
			ret = -1;
			break;
		}
	} else if (!strncmp(intf->name, "Bob", 3)) {
		switch (ram_code) {
		case 3:
			gru_dimm_count = 2;
			gru_mem_info = &samsung_lpddr3_k4e8e324eb_egcf;
			break;
		case 4:
			gru_dimm_count = 2;
			gru_mem_info = &micron_lpddr3_mt52l256m32d1pf_107wtb;
			break;
		case 5:
			gru_dimm_count = 2;
			gru_mem_info = &samsung_lpddr3_k4e6e304eb_egcf;
			break;
		case 6:
			gru_dimm_count = 2;
			gru_mem_info = &micron_lpddr3_mt52l512m32d2pf_107wtb;
			break;
		default:
			ret = -1;
			break;
		}
	} else {
		ret = -1;
	}

	done = 1;
	return ret;
}

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_count(struct platform_intf *intf)
{
	if (read_ram_code(intf) < 0)
		return -1;

	return gru_dimm_count;
}

static int get_mem_info(struct platform_intf *intf,
			const struct nonspd_mem_info **info)
{
	if (read_ram_code(intf) < 0)
		return -1;

	*info = gru_mem_info;
	return 0;
}

struct memory_cb gru_memory_cb = {
	.dimm_count		= &dimm_count,
	.nonspd_mem_info	= &get_mem_info,
};
