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
#define FDT_BOARD_ID_PATH	"firmware/coreboot/board-id"

static uint32_t fdt_get_uint32_val(const char *path)
{
	static uint32_t ret = ~0;	/* init to invalid value */
	uint32_t tmp;
	int fd;
	struct string_builder *sb;

	sb = new_string_builder();
	string_builder_strcat(sb, mosys_get_root_prefix());
	string_builder_strcat(sb, FDT_ROOT);
	string_builder_strcat(sb, path);

	fd = file_open(string_builder_get_string(sb), FILE_READ);
	if (fd < 0) {
		lprintf(LOG_ERR, "Unable to open %s.\n",
				string_builder_get_string(sb));
		goto out_1;
	}

	if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp)) {
		lprintf(LOG_ERR, "Failed to read devicetree node.\n");
		goto out_2;
	}

	tmp = ntohl(tmp);
	ret = tmp;

out_2:
	close(fd);
out_1:
	free_string_builder(sb);
	return ret;
}

/*
 * fdt_get_ram_code - Obtain RAM code from FDT ram-code node
 *
 * returns ram code if successful, ~0 (invalid) to indicate failure
 */
uint32_t fdt_get_ram_code(void)
{
	static int done = 0;
	static uint32_t ret = 0;

	if (done)
		return ret;

	ret = fdt_get_uint32_val(FDT_RAM_CODE_PATH);
	if (ret == 0xffffffff)
		lprintf(LOG_ERR, "%s: ram_code is invalid.\n", __func__);
	else
		lprintf(LOG_DEBUG, "%s: ram_code: %u\n", __func__, ret);

	done = 1;
	return ret;
}

/*
 * fdt_get_board_id - Obtain board ID code from FDT board-id node
 *
 * returns board ID if successful, ~0 (invalid) to indicate failure
 */
uint32_t fdt_get_board_id(void)
{
	static int done = 0;
	static uint32_t ret = 0;

	if (done)
		return ret;

	ret = fdt_get_uint32_val(FDT_BOARD_ID_PATH);
	if (ret == 0xffffffff)
		lprintf(LOG_ERR, "%s: board_id is invalid.\n", __func__);
	else
		lprintf(LOG_DEBUG, "%s: board_id: %u\n", __func__, ret);

	done = 1;
	return ret;
}
