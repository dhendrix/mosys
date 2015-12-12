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
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/flashrom.h"
#include "lib/spd.h"
#include "lib/smbios.h"
#include "lib/smbios_tables.h"

#include "cyan.h"

/*
 * cyan_dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int cyan_dimm_count(struct platform_intf *intf)
{
	int status = 0, dimm_cnt = 0;
	struct smbios_table table;

	while (status == 0) {
		status = smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm_cnt,
					   &table,
					   SMBIOS_LEGACY_ENTRY_BASE,
					   SMBIOS_LEGACY_ENTRY_LEN);
		if (status == 0)
			dimm_cnt++;
	}
	return dimm_cnt;
}

static int cyan_spd_read(struct platform_intf *intf,
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

int cyan_dimm_speed(struct platform_intf *intf,
		     int dimm, struct kv_pair *kv)
{
	struct smbios_table table;
	if (smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		return -1;
	}

	kv_pair_fmt(kv, "speed", "%d MHz", table.data.mem_device.speed);

	return 0;
}

static struct memory_spd_cb cyan_spd_cb = {
	.read		= cyan_spd_read,
};

struct memory_cb cyan_memory_cb = {
	.dimm_count	= cyan_dimm_count,
	.dimm_speed	= cyan_dimm_speed,
	.spd		= &cyan_spd_cb,
};
