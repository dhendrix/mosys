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

#include "mosys/callbacks.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/gpio.h"

#include "lib/flashrom.h"
#include "lib/spd.h"
#include "lib/smbios.h"

#include "glados.h"

static int glados_spd_read(struct platform_intf *intf,
		 int dimm, int reg, int spd_len, uint8_t *spd_buf)
{
	static uint8_t *fw_buf;
	static int fw_size = 0;

	/* dimm cnt is 0 based */
	if (dimm >= intf->cb->memory->dimm_count(intf)) {
		lprintf(LOG_DEBUG, "%s: Invalid DIMM specified\n", __func__);
		return -1;
	}

	if (fw_size < 0)
		return -1;	/* previous attempt failed */

	if (!fw_size) {
		fw_size = flashrom_read_host_firmware_region(intf, &fw_buf);
		if (fw_size < 0)
			return -1;
		add_destroy_callback(free, fw_buf);
	}

	return spd_read_from_cbfs(intf, dimm, reg,
				spd_len, spd_buf, fw_size, fw_buf);
}

static struct memory_spd_cb glados_spd_cb = {
	.read		= glados_spd_read,
};

struct memory_cb glados_memory_cb = {
	.dimm_count	= smbios_dimm_count,
	.dimm_speed	= smbios_dimm_speed,
	.spd		= &glados_spd_cb,
};
