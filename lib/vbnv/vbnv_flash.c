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

#include <fmap.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <valstr.h>

#include "lib/eeprom.h"

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/output.h"
#include "mosys/platform.h"

 /* VBNV entries are 16 bytes each and are stored back-to-back in RW_NVRAM. */
#define VBNV_BLOCK_SIZE	16

/*
 * vbnv_find_eeprom - determine which eeprom and region contains VBNV data.
 *
 * @intf:	platform interface
 * @eeprom_p:	double-pointer to eeprom struct (used as an output)
 * @region_p:	double-pointer to region struct (used as an output)
 *
 * This function iterates thru the platform's list of eeproms to find which
 * one has VBNV data and sets the outputs accordingly.
 *
 * returns -1 to indicate failure, 0 to indicate success
 */
static int vbnv_find_eeprom(struct platform_intf *intf,
		struct eeprom **eeprom_p, struct eeprom_region **region_p)
{
	struct eeprom *eeprom;
	struct eeprom_region *region = NULL;

	if (!intf->cb->eeprom || !intf->cb->eeprom->eeprom_list)
		return -1;

	*eeprom_p = NULL;
	*region_p = NULL;

	/* Find eeprom containing VBNV, if any. */
	for (eeprom = &intf->cb->eeprom->eeprom_list[0];
			eeprom->name; eeprom++) {
		/* TODO: for now, assume VBNV will be found using fmap */
		if (!(eeprom->flags & EEPROM_FLAG_FMAP))
			continue;

		if (!eeprom->regions)
			continue;

		for (region = &eeprom->regions[0]; region->flag; region++) {
			if (region->flag & EEPROM_FLAG_VBNV) {
				lprintf(LOG_DEBUG, "%s: Found region->name: "
					"%s\n",	__func__, region->name);
				*eeprom_p = eeprom;
				*region_p = region;
				break;
			}
		}

		if (*eeprom_p)
			break;
	}

	if (!(*eeprom_p)) {
		lprintf(LOG_WARNING, "No ROM found with a VBNV region.\n");
		return -1;
	}

	if (region->len % VBNV_BLOCK_SIZE != 0) {
		lprintf(LOG_WARNING, "VBNV region size (%u) is not "
				"divisible by block size (%u).\n",
				region->len, VBNV_BLOCK_SIZE);
	}

	return 0;
}

/* return index of last valid vbnv entry */
static int vbnv_index(const uint8_t *data, size_t len)
{
	int index;
	uint8_t blank[VBNV_BLOCK_SIZE];

	memset(blank, 0xff, VBNV_BLOCK_SIZE);
	for (index = 0; index < len / VBNV_BLOCK_SIZE; index++) {
		unsigned int offset = index * VBNV_BLOCK_SIZE;

		if (!memcmp(blank, &data[offset], VBNV_BLOCK_SIZE)) {
			lprintf(LOG_DEBUG, "Found blank VBNV block (index: %d, "
					"offset: 0x%0x)\n", index, offset);
			break;
		}
	}

	if (index == 0) {
		lprintf(LOG_ERR, "VBNV is uninitialized\n");
		return -1;
	} else if (index >= len) {
		lprintf(LOG_ERR, "VBNV is full\n");
		return -1;
	} else {
		index--;
	}

	return index;
}

/*
 * vbnv_fetch_from_flash - fetch the vbnv content from the flash.
 *
 * @intf:          platform interface used for low level hardware access
 * @block:         pointer to buffer to copy vboot block into
 *
 * returns -1 on failure, 0 on success
 */
static int vbnv_fetch_from_flash(struct platform_intf *intf, uint8_t *block)
{
	struct eeprom *eeprom = NULL;
	struct eeprom_region *region = NULL;
	uint8_t *data;
	int ret = -1, bytes_read, index;

	if (vbnv_find_eeprom(intf, &eeprom, &region))
		return -1;

	bytes_read = eeprom->device->read_by_name(intf, eeprom,
						region->name, &data);
	if (bytes_read < 0) {
		lprintf(LOG_WARNING, "Failed to read VBNV from flash.\n");
		goto vbnv_fetch_from_flash_exit;
	}

	if ((index = vbnv_index(data, bytes_read)) < 0)
		goto vbnv_fetch_from_flash_exit;

	lprintf(LOG_DEBUG, "Using VBNV block at index %d\n", index);
	memcpy(block, &data[index * VBNV_BLOCK_SIZE], VBNV_BLOCK_SIZE);
	ret = 0;

vbnv_fetch_from_flash_exit:
	if (data)
		free(data);
	return ret;
}

int vbnv_flash_vboot_read(struct platform_intf *intf)
{
	struct kv_pair *kv;
	uint8_t block[VBNV_BLOCK_SIZE];
	char hexstring[VBNV_BLOCK_SIZE * 2 + 1];
	int i, rc;

	if (vbnv_fetch_from_flash(intf, block))
		return -1;

	for (i = 0; i < VBNV_BLOCK_SIZE; i++)
		snprintf(hexstring + i * 2, 3, "%02x", block[i]);

	kv = kv_pair_new();
	kv_pair_fmt(kv, "vbnvcontext", "%s", hexstring);
	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

/*
 * vbnv_write_to_flash - write vbnv data to flash
 *
 * @intf:	platform interface used for low level hardware access
 * @data:	vbnv data to write
 *
 * returns -1 on failure, 0 on success
 */
static int vbnv_write_to_flash(struct platform_intf *intf, const void *data)
{
	struct eeprom *eeprom = NULL;
	struct eeprom_region *region = NULL;
	uint8_t *buf;
	int ret = -1, len, index;

	if (vbnv_find_eeprom(intf, &eeprom, &region))
		return -1;

	len = eeprom->device->read_by_name(intf, eeprom,
						region->name, &buf);
	if (len < 0) {
		lprintf(LOG_WARNING, "Failed to read VBNV from flash.\n");
		goto vbnv_write_to_flash_exit;
	}

	if ((index = vbnv_index(buf, len)) < 0)
		goto vbnv_write_to_flash_exit;


	index++;
	if (index * VBNV_BLOCK_SIZE >= len) {
		lprintf(LOG_DEBUG, "%s full, clearing.\n", region->name);
		memset(buf, 0xff, len);
		memcpy(&buf[0], data, VBNV_BLOCK_SIZE);
		index = 0;
	} else {
		memcpy(&buf[index * VBNV_BLOCK_SIZE],
				data, VBNV_BLOCK_SIZE);
	}

	lprintf(LOG_DEBUG, "Writing VBNV block at index %d\n", index);
	if (eeprom->device->write_by_name(intf, eeprom,
				region->name, len, buf) != len) {
		lprintf(LOG_WARNING, "Failed to read VBNV from flash.\n");
		goto vbnv_write_to_flash_exit;
	}

	ret = 0;
vbnv_write_to_flash_exit:
	if (buf)
		free(buf);
	return ret;
}

int vbnv_flash_vboot_write(struct platform_intf *intf, const char *hexstring)
{
	uint8_t block[VBNV_BLOCK_SIZE];
	char hexdigit[3];
	int i, len;

	len = strlen(hexstring);
	if (len != VBNV_BLOCK_SIZE * 2) {
		lprintf(LOG_DEBUG, "%s: hexstring's length must "
				   "be %d (got %d)\n",
				   __func__, VBNV_BLOCK_SIZE * 2, len);
		return -1;
	}
	len /= 2;

	hexdigit[2] = '\0';
	for (i = 0; i < len; i++) {
		hexdigit[0] = hexstring[i * 2];
		hexdigit[1] = hexstring[i * 2 + 1];
		block[i] = strtol(hexdigit, NULL, 16);
	}

	return vbnv_write_to_flash(intf, block);
}

struct nvram_cb cros_spi_flash_nvram_cb = {
	.vboot_read	= vbnv_flash_vboot_read,
	.vboot_write	= vbnv_flash_vboot_write,
};
