/* Copyright 2014, Google Inc.
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
 *
 * fdt.c: Helper functions for getting data out of the device tree.
 */

#include <arpa/inet.h>
#include <inttypes.h>
#include <unistd.h>

#include "lib/fdt.h"
#include "lib/file.h"
#include "lib/string_builder.h"

#include "mosys/globals.h"
#include "mosys/log.h"

#define FDT_ROOT		"/proc/device-tree/"
/* FIXME: assume coreboot for now */
#define FDT_RAM_CODE_PATH	"firmware/coreboot/ram-code"

/*
 * fdt_get_ram_code - Obtain RAM code from FDT ram-code node
 *
 * returns ram code if successful, ~0 (invalid) to indicate failure
 */
uint32_t fdt_get_ram_code(void)
{
	static int done = 0;
	static uint32_t ret = ~(0);	/* init to invalid value */
	uint32_t tmp;
	int fd;
	struct string_builder *sb;

	if (done)
		return ret;

	sb = new_string_builder();
	string_builder_strcat(sb, mosys_get_root_prefix());
	string_builder_strcat(sb, FDT_ROOT);
	string_builder_strcat(sb, FDT_RAM_CODE_PATH);

	fd = file_open(string_builder_get_string(sb), FILE_READ);
	if (fd < 0) {
		lprintf(LOG_ERR, "Unable to open RAM code devicetree node.\n");
		goto fdt_ram_code_done;
	}

	if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
		lprintf(LOG_ERR, "Failed to read ram-code.\n");
		goto fdt_ram_code_done;
	}

	tmp = ntohl(tmp);
	lprintf(LOG_DEBUG, "%s: ram_code: %u\n", __func__, tmp);
	ret = tmp;

fdt_ram_code_done:
	close(fd);
	free_string_builder(sb);
	done = 1;
	return ret;
}
