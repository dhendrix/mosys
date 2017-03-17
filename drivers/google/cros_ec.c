/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
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
 * gec.c: Generic CrOS EC functions and structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/cros_ec.h"
#include "drivers/google/cros_ec_commands.h"
#include "drivers/google/cros_ec_dev.h"

#include "lib/math.h"

#define BOARD_VERSION_LEN	8	/* a 16-bit value or "Unknown" */
#define ANX74XX_VENDOR_ID	0xAAAA
#define PS8751_VENDOR_ID	0x1DA0

int cros_ec_hello(struct platform_intf *intf, struct ec_cb *ec)
{
	struct ec_params_hello p;
	struct ec_response_hello r;
	int rv;
	struct cros_ec_priv *priv;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	p.in_data = 0xa0b0c0d0;

	rv = priv->cmd(intf, ec, EC_CMD_HELLO, 0, &p,
		       sizeof(p), &r, sizeof(r));
	if (rv)
		return rv;

	if (r.out_data != 0xa1b2c3d4) {
		lprintf(LOG_ERR, "Expected response 0x%08x, got 0x%08x\n",
			0xa1b2c3d4, r.out_data);
		rv = -1;
	}

	lprintf(LOG_DEBUG, "%s: EC says hello!\n", __func__);
	return rv;
}

const char *cros_ec_version(struct platform_intf *intf, struct ec_cb *ec)
{
	static const char *const fw_copies[] = { "unknown", "RO", "RW" };
	struct ec_response_get_version r;
	const char *ret = NULL;
	struct cros_ec_priv *priv;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	if (priv->cmd(intf, ec, EC_CMD_GET_VERSION, 0, &r, sizeof(r), NULL, 0))
		return NULL;

	/* Ensure versions are null-terminated before we print them */
	r.version_string_ro[sizeof(r.version_string_ro) - 1] = '\0';
	r.version_string_rw[sizeof(r.version_string_rw) - 1] = '\0';

	/* Print versions */
	lprintf(LOG_DEBUG, "RO version:    %s\n", r.version_string_ro);
	lprintf(LOG_DEBUG, "RW version:    %s\n", r.version_string_rw);
	lprintf(LOG_DEBUG, "Firmware copy: %s\n",
	       (r.current_image < ARRAY_SIZE(fw_copies) ?
		fw_copies[r.current_image] : "?"));

	switch (r.current_image) {
	case EC_IMAGE_RO:
		ret = mosys_strdup(r.version_string_ro);
		break;
	case EC_IMAGE_RW:
		ret = mosys_strdup(r.version_string_rw);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: cannot determine version\n", __func__);
		break;
	}

	return ret;
}

int cros_ec_board_version(struct platform_intf *intf, struct ec_cb *ec)
{
	struct cros_ec_priv *priv;
	struct ec_response_board_version r;
	int rc = 0;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	rc = priv->cmd(intf, ec, EC_CMD_GET_BOARD_VERSION, 0,
		       &r, sizeof(r), NULL, 0);
	if (rc)
		return -1;

	lprintf(LOG_DEBUG, "CrOS EC Board Version: %d\n", r.board_version);

	return r.board_version;
}

char *cros_ec_board_version_str(struct platform_intf *intf)
{
	int version;
	char *str = mosys_zalloc(BOARD_VERSION_LEN);

	/* Baseboard may have variants without EC in which case this
	   function needs to be used more selectively. */
	MOSYS_CHECK(intf && intf->cb->ec);

	version = cros_ec_board_version(intf, intf->cb->ec);
	if (version < 0) {
		snprintf(str, BOARD_VERSION_LEN, "Unknown");
	} else {
		/*
		 * Prepend "rev" to match format currently used for CrOS
		 * platforms with devicetree and newer (beginning with Reef)
		 * SMBIOS system (type-1) version format.
		 */
		snprintf(str, BOARD_VERSION_LEN, "rev%d", version);
	}

	return str;
}

int cros_ec_chip_info(struct platform_intf *intf, struct ec_cb *ec,
		  struct ec_response_get_chip_info *info)
{
	int rc = 0;
	struct cros_ec_priv *priv;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	rc = priv->cmd(intf, ec,EC_CMD_GET_CHIP_INFO, 0,
		       info, sizeof(*info), NULL, 0);
	if (rc)
		return rc;

	lprintf(LOG_DEBUG, "CrOS EC vendor: %s\n", info->vendor);
	lprintf(LOG_DEBUG, "CrOS EC name: %s\n", info->name);
	lprintf(LOG_DEBUG, "CrOS EC revision: %s\n", info->revision);

	return rc;
}

int cros_ec_pd_chip_info(struct platform_intf *intf, struct ec_cb *ec, int port)
{
	struct cros_ec_priv *priv;
	struct ec_params_pd_chip_info p;
	struct ec_response_pd_chip_info info;
	struct kv_pair *kv;
	int rc;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	p.port = port;
	p.renew = 0;

	rc = priv->cmd(intf, ec, EC_CMD_PD_CHIP_INFO, 0,
		       &info, sizeof(info), &p, sizeof(p));
	if (rc)
		return rc;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "vendor_id", "0x%x", info.vendor_id);
	kv_pair_fmt(kv, "product_id", "0x%x", info.product_id);
	kv_pair_fmt(kv, "device_id", "0x%x", info.device_id);
	switch (info.vendor_id) {
	case ANX74XX_VENDOR_ID:
	case PS8751_VENDOR_ID:
		kv_pair_fmt(kv, "fw_version", "0x%" PRIx64,
			    info.fw_version_number);
		break;
	default:
		kv_pair_fmt(kv, "fw_version", "UNSUPPORTED");
	}
	kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

int cros_ec_flash_info(struct platform_intf *intf, struct ec_cb *ec,
		   struct ec_response_flash_info *info)
{
	int rc = 0;
	struct cros_ec_priv *priv;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	rc = priv->cmd(intf, ec, EC_CMD_FLASH_INFO, 0,
		       info, sizeof(*info), NULL, 0);
	if (rc)
		return rc;

	lprintf(LOG_DEBUG, "CrOS EC flash size: 0x%06x\n", info->flash_size);
	lprintf(LOG_DEBUG, "CrOS EC write block size: 0x%06x\n",
			info->write_block_size);
	lprintf(LOG_DEBUG, "CrOS EC erase block size: 0x%06x\n",
			info->erase_block_size);
	lprintf(LOG_DEBUG, "CrOS EC protection block size: 0x%06x\n",
			info->protect_block_size);

	return rc;
}

int cros_ec_get_firmware_rom_size(struct platform_intf *intf)
{
	struct ec_response_flash_info info;

	if (cros_ec_flash_info(intf, intf->cb->ec, &info) < 0) {
		lprintf(LOG_ERR, "%s: Failed to obtain flash info\n", __func__);
		return -1;
	}

	return info.flash_size;
}

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_ec_detect(struct platform_intf *intf, struct ec_cb *ec)
{
	struct ec_params_hello request;
	struct ec_response_hello response;
	int result = 0;
	int ret = 0;
	struct cros_ec_priv *priv;

	if (!intf->cb || !ec || !ec->priv)
		return -1;
	priv = ec->priv;

	/* Say hello to EC. */
	request.in_data = 0xf0e0d0c0;  /* Expect EC will add on 0x01020304. */

	lprintf(LOG_DEBUG, "%s: sending HELLO request with 0x%08x\n",
		__func__, request.in_data);
	result  = priv->cmd(intf, ec, EC_CMD_HELLO, 0,
			    &response, sizeof(response),
			    &request, sizeof(request));
	lprintf(LOG_DEBUG, "%s: response: 0x%08x\n",
		__func__, response.out_data);

	if (result || response.out_data != 0xf1e2d3c4) {
		lprintf(LOG_DEBUG, "response.out_data is not 0xf1e2d3c4.\n"
			"result=%d, request=0x%x response=0x%x\n",
		        result, request.in_data, response.out_data);
	} else {
		ret = 1;
	}

	return ret;
}

int cros_ec_vbnvcontext_read(struct platform_intf *intf, struct ec_cb *ec,
			     uint8_t *block)
{
	struct ec_params_vbnvcontext request;
	struct cros_ec_priv *priv;
	int result;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	lprintf(LOG_DEBUG, "%s: sending VBNV_CONTEXT read request\n",
		__func__);
	request.op = EC_VBNV_CONTEXT_OP_READ;
	result = priv->cmd(intf, ec, EC_CMD_VBNV_CONTEXT, EC_VER_VBNV_CONTEXT,
			   block, EC_VBNV_BLOCK_SIZE,
			   &request, sizeof(request.op));
	if (result) {
		lprintf(LOG_DEBUG, "%s: result=%d\n", __func__, result);
		return -1;
	}

	return 0;
}

int cros_ec_vbnvcontext_write(struct platform_intf *intf, struct ec_cb *ec,
			      const uint8_t *block)
{
	struct ec_params_vbnvcontext request;
	struct cros_ec_priv *priv;
	int result;

	MOSYS_CHECK(ec && ec->priv);
	priv = ec->priv;

	lprintf(LOG_DEBUG, "%s: sending VBNV_CONTEXT write request\n",
		__func__);
	request.op = EC_VBNV_CONTEXT_OP_WRITE;
	memcpy(request.block, block, sizeof(request.block));
	result = priv->cmd(intf, ec, EC_CMD_VBNV_CONTEXT, EC_VER_VBNV_CONTEXT,
			   NULL, 0,
			   &request, sizeof(request));
	if (result) {
		lprintf(LOG_DEBUG, "%s: result=%d\n", __func__, result);
		return -1;
	}

	return 0;
}

int cros_ec_vboot_read(struct platform_intf *intf)
{
	struct kv_pair *kv;
	uint8_t block[EC_VBNV_BLOCK_SIZE];
	char hexstring[EC_VBNV_BLOCK_SIZE * 2 + 1];
	int i, rc;

	if (cros_ec_vbnvcontext_read(intf, intf->cb->ec, block))
		return -1;

	for (i = 0; i < EC_VBNV_BLOCK_SIZE; i++)
		snprintf(hexstring + i * 2, 3, "%02x", block[i]);

	kv = kv_pair_new();
	kv_pair_fmt(kv, "vbnvcontext", "%s", hexstring);
	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

int cros_ec_vboot_write(struct platform_intf *intf, const char *hexstring)
{
	uint8_t block[EC_VBNV_BLOCK_SIZE];
	char hexdigit[3];
	int i, len;

	len = strlen(hexstring);
	if (len != EC_VBNV_BLOCK_SIZE * 2) {
		lprintf(LOG_DEBUG, "%s: hexstring's length must "
				   "be %d (got %d)\n",
				   __func__, EC_VBNV_BLOCK_SIZE * 2, len);
		return -1;
	}
	len /= 2;

	hexdigit[2] = '\0';
	for (i = 0; i < len; i++) {
		hexdigit[0] = hexstring[i * 2];
		hexdigit[1] = hexstring[i * 2 + 1];
		block[i] = strtol(hexdigit, NULL, 16);
	}

	return cros_ec_vbnvcontext_write(intf, intf->cb->ec, block);
}

struct nvram_cb cros_ec_nvram_cb = {
	.vboot_read	= cros_ec_vboot_read,
	.vboot_write	= cros_ec_vboot_write,
};

int cros_ec_setup(struct platform_intf *intf)
{
	MOSYS_CHECK(intf->cb && intf->cb->ec);

	lprintf(LOG_DEBUG, "%s: Trying devfs interface...\n", __func__);
	if (cros_ec_setup_dev(intf) == 1)
		return 1;

	return 0;
}

int cros_pd_setup(struct platform_intf *intf)
{
	MOSYS_CHECK(intf->cb && intf->cb->pd);

	lprintf(LOG_DEBUG, "%s: Trying devfs interface...\n", __func__);
	if (cros_pd_setup_dev(intf) == 1)
		return 1;

	return 0;
}

int cros_fp_setup(struct platform_intf *intf)
{
	MOSYS_CHECK(intf->cb && intf->cb->fp);

	lprintf(LOG_DEBUG, "%s: Trying devfs interface...\n", __func__);
	if (cros_fp_setup_dev(intf) == 1)
		return 1;

	return 0;
}
