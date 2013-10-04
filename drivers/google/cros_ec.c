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

#include "lib/math.h"

int cros_ec_hello(struct platform_intf *intf)
{
	struct ec_params_hello p;
	struct ec_response_hello r;
	int rv;
	struct cros_ec_priv *priv = intf->cb->ec->priv;

	p.in_data = 0xa0b0c0d0;

	rv = priv->cmd(intf, EC_CMD_HELLO, 0, &p,
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

const char *cros_ec_version(struct platform_intf *intf)
{
	static const char *const fw_copies[] = { "unknown", "RO", "RW" };
	struct ec_response_get_version r;
	const char *ret = NULL;
	struct cros_ec_priv *priv = intf->cb->ec->priv;

	if (priv->cmd(intf, EC_CMD_GET_VERSION, 0, &r, sizeof(r), NULL, 0))
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

int cros_ec_chip_info(struct platform_intf *intf,
		  struct ec_response_get_chip_info *info)
{
	int rc = 0;
	struct cros_ec_priv *priv = intf->cb->ec->priv;

	rc = priv->cmd(intf, EC_CMD_GET_CHIP_INFO, 0,
		       info, sizeof(*info), NULL, 0);
	if (rc)
		return rc;

	lprintf(LOG_DEBUG, "CrOS EC vendor: %s\n", info->vendor);
	lprintf(LOG_DEBUG, "CrOS EC name: %s\n", info->name);
	lprintf(LOG_DEBUG, "CrOS EC revision: %s\n", info->revision);

	return rc;
}

int cros_ec_flash_info(struct platform_intf *intf,
		   struct ec_response_flash_info *info)
{
	int rc = 0;
	struct cros_ec_priv *priv = intf->cb->ec->priv;

	rc = priv->cmd(intf, EC_CMD_FLASH_INFO, 0,
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

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_ec_detect(struct platform_intf *intf)
{
	struct ec_params_hello request;
	struct ec_response_hello response;
	int result = 0;
	int ret = 0;
	struct cros_ec_priv *priv;

	if (!intf->cb || !intf->cb->ec || !intf->cb->ec->priv)
		return -1;
	priv = intf->cb->ec->priv;

	/* Say hello to EC. */
	request.in_data = 0xf0e0d0c0;  /* Expect EC will add on 0x01020304. */

	lprintf(LOG_DEBUG, "%s: sending HELLO request with 0x%08x\n",
		__func__, request.in_data);
	result  = priv->cmd(intf, EC_CMD_HELLO, 0,
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

int cros_ec_vbnvcontext_read(struct platform_intf *intf, uint8_t *block)
{
	struct ec_params_vbnvcontext request;
	struct cros_ec_priv *priv;
	int result;

	if (!intf->cb || !intf->cb->ec || !intf->cb->ec->priv)
		return -1;
	priv = intf->cb->ec->priv;

	lprintf(LOG_DEBUG, "%s: sending VBNV_CONTEXT read request\n",
		__func__);
	request.op = EC_VBNV_CONTEXT_OP_READ;
	result = priv->cmd(intf, EC_CMD_VBNV_CONTEXT, EC_VER_VBNV_CONTEXT,
			   block, EC_VBNV_BLOCK_SIZE,
			   &request, sizeof(request.op));
	if (result) {
		lprintf(LOG_DEBUG, "%s: result=%d\n", __func__, result);
		return -1;
	}

	return 0;
}

int cros_ec_vbnvcontext_write(struct platform_intf *intf, const uint8_t *block)
{
	struct ec_params_vbnvcontext request;
	struct cros_ec_priv *priv;
	int result;

	if (!intf->cb || !intf->cb->ec || !intf->cb->ec->priv)
		return -1;
	priv = intf->cb->ec->priv;

	lprintf(LOG_DEBUG, "%s: sending VBNV_CONTEXT write request\n",
		__func__);
	request.op = EC_VBNV_CONTEXT_OP_WRITE;
	memcpy(request.block, block, sizeof(request.block));
	result = priv->cmd(intf, EC_CMD_VBNV_CONTEXT, EC_VER_VBNV_CONTEXT,
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

	if (cros_ec_vbnvcontext_read(intf, block))
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

	return cros_ec_vbnvcontext_write(intf, block);
}

struct nvram_cb cros_ec_nvram_cb = {
	.vboot_read	= cros_ec_vboot_read,
	.vboot_write	= cros_ec_vboot_write,
};
