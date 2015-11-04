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

#include "lib/nonspd.h"

#include "mosys/log.h"
#include "mosys/platform.h"

#include "storm.h"

/* Treat each module as a logical "DIMM" */
#define STORM_DIMM_COUNT	2

enum storm_memory_config {
	SAMSUNG_DDR3_1600_1G,
	MICRON_DDR3L_1600_1G,
	MEM_UNKNOWN,
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
	if (!strcmp(intf->name, "Storm"))
		return SAMSUNG_DDR3_1600_1G;
	else if (!strcmp(intf->name, "Whirlwind"))
		return MICRON_DDR3L_1600_1G;
	else if (!strcmp(intf->name, "Arkham"))
		return MICRON_DDR3L_1600_1G;
	return MEM_UNKNOWN;
}

static int get_mem_info(struct platform_intf *intf,
			const struct nonspd_mem_info **info)
{
	switch (get_memory_config(intf)) {
	case SAMSUNG_DDR3_1600_1G:
		*info = &samsung_k4b4g1646d;
		break;
	case MICRON_DDR3L_1600_1G:
		*info = &micron_mt41k256m16ha;
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
