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
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "lib/fdt.h"
#include "lib/file.h"
#include "lib/string_builder.h"

#include "mosys/globals.h"
#include "mosys/log.h"

/* for fdt_set_nvram_cb() */
extern struct nvram_cb cros_ec_nvram_cb;
extern struct nvram_cb cros_spi_flash_nvram_cb;

#define FDT_ROOT		"/proc/device-tree/"
#define FDT_VBNV_STORAGE_PATH	"firmware/chromeos/nonvolatile-context-storage"
/* FIXME: assume coreboot for now */
#define FDT_RAM_CODE_PATH	"firmware/coreboot/ram-code"
#define FDT_BOARD_ID_PATH	"firmware/coreboot/board-id"


/* returns number of bytes read or -1 to indicate error */
static int fdt_read_node(const char *path, char *buf, int len)
{
	int fd, ret;
	struct string_builder *sb;

	sb = new_string_builder();
	string_builder_strcat(sb, mosys_get_root_prefix());
	string_builder_strcat(sb, FDT_ROOT);
	string_builder_strcat(sb, path);

	fd = file_open(string_builder_get_string(sb), FILE_READ);
	if (fd < 0) {
		ret = fd;
		lprintf(LOG_ERR, "Unable to open %s.\n",
				string_builder_get_string(sb));
		goto out_1;
	}

	ret = read(fd, buf, len);
	if (ret < 0) {
		lprintf(LOG_ERR, "Failed to read devicetree node.\n");
		goto out_2;
	}

out_2:
	close(fd);
out_1:
	free_string_builder(sb);
	return ret;
}

static int fdt_get_uint32_val(const char *path, uint32_t *val)
{
	int len = sizeof(*val);

	if (fdt_read_node(path, (char *)val, len) != len)
		return -1;

	*val = ntohl(*val);
	return 0;
}

/*
 * fdt_get_ram_code - Obtain RAM code from FDT ram-code node
 *
 * returns 0 to indicate success, <0 to indicate failure.
 */
int fdt_get_ram_code(uint32_t *ram_code)
{
	if (fdt_get_uint32_val(FDT_RAM_CODE_PATH, ram_code) < 0) {
		lprintf(LOG_ERR, "%s: Error when reading RAM code\n", __func__);
		return -1;
	}

	if (*ram_code == 0xffffffff) {
		lprintf(LOG_ERR, "%s: ram_code is invalid.\n", __func__);
		return -1;
	}

	lprintf(LOG_DEBUG, "%s: ram_code: %u\n", __func__, *ram_code);
	return 0;
}

/*
 * fdt_get_board_id - Obtain board ID code from FDT board-id node
 *
 * returns 0 to indicate success, <0 to indicate failure.
 */
int fdt_get_board_id(uint32_t *board_id)
{
	if (fdt_get_uint32_val(FDT_BOARD_ID_PATH, board_id) < 0) {
		lprintf(LOG_ERR, "%s: Error when reading board ID\n", __func__);
		return -1;
	}

	if (*board_id == 0xffffffff) {
		lprintf(LOG_ERR, "%s: board_id is invalid.\n", __func__);
		return -1;
	}

	lprintf(LOG_DEBUG, "%s: board_id: %u\n", __func__, *board_id);
	return 0;
}

static enum vbnv_storage_media fdt_get_vbnv_storage(void)
{
	char buf[8];
	int len = sizeof(buf);
	enum vbnv_storage_media ret;

	if (fdt_read_node(FDT_VBNV_STORAGE_PATH, buf, len) < 0) {
		lprintf(LOG_ERR, "%s: Error when reading VBNV storage"
				"type\n", __func__);
		return VBNV_STORAGE_UNKNOWN;
	}

	if (!strncmp(buf, "cros-ec", len) || !strncmp(buf, "mkbp", len))
		ret = VBNV_STORAGE_CROS_EC;
	else if (!strncmp(buf, "disk", len))
		ret = VBNV_STORAGE_DISK;
	else if (!strncmp(buf, "flash", len))
		ret = VBNV_STORAGE_FLASH;
	else if (!strncmp(buf, "nvram", len))
		ret = VBNV_STORAGE_NVRAM;
	else
		ret = VBNV_STORAGE_UNKNOWN;

	lprintf(LOG_DEBUG, "%s: VBNV storage type: %d\n", __func__, ret);
	return ret;
}

/*
 * fdt_set_nvram_cb - Set platform's nvram callbacks based on FDT
 *
 * returns 0 to indicate success, <0 to indicate failure.
 */
int fdt_set_nvram_cb(struct platform_intf *intf)
{
	switch (fdt_get_vbnv_storage()) {
	case VBNV_STORAGE_CROS_EC:
		intf->cb->nvram = &cros_ec_nvram_cb;
		break;
	case VBNV_STORAGE_DISK:
		lprintf(LOG_ERR, "VBNV from disk is not supported yet.\n");
		return -ENOSYS;
	case VBNV_STORAGE_FLASH:
		intf->cb->nvram = &cros_spi_flash_nvram_cb;
		break;
	case VBNV_STORAGE_NVRAM:
		return -ENOSYS;
	default:
		return -1;
	}

	return 0;
}
