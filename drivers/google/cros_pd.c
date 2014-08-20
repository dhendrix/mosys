/*
 * Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
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

static int set_device_index(struct platform_intf *intf, int index)
{
	struct cros_ec_priv *ec_priv = intf->cb->ec->priv;
	int orig_device_index = ec_priv->device_index;
	ec_priv->device_index = index;
	return orig_device_index;
}

int cros_pd_hello(struct platform_intf *intf)
{
	struct cros_ec_priv *priv = intf->cb->pd->priv;
	int index, ret;

	index = set_device_index(intf, priv->device_index);
	ret = cros_ec_hello(intf);
	set_device_index(intf, index);

	return ret;
}

const char *cros_pd_version(struct platform_intf *intf)
{
	struct cros_ec_priv *priv = intf->cb->pd->priv;
	int index;
	const char *ret;

	index = set_device_index(intf, priv->device_index);
	ret = cros_ec_version(intf);
	set_device_index(intf, index);

	return ret;
}

int cros_pd_board_version(struct platform_intf *intf)
{
	struct cros_ec_priv *priv = intf->cb->pd->priv;
	int index, ret;

	index = set_device_index(intf, priv->device_index);
	ret = cros_ec_board_version(intf);
	set_device_index(intf, index);

	return ret;
}

int cros_pd_chip_info(struct platform_intf *intf,
		      struct ec_response_get_chip_info *info)
{
	struct cros_ec_priv *priv = intf->cb->pd->priv;
	int index, ret;

	index = set_device_index(intf, priv->device_index);
	ret = cros_ec_chip_info(intf, info);
	set_device_index(intf, index);

	return ret;
}

int cros_pd_flash_info(struct platform_intf *intf,
		       struct ec_response_flash_info *info)
{
	struct cros_ec_priv *priv = intf->cb->pd->priv;
	int index, ret;

	index = set_device_index(intf, priv->device_index);
	ret = cros_ec_flash_info(intf, info);
	set_device_index(intf, index);

	return ret;
}

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_pd_detect(struct platform_intf *intf)
{
	struct cros_ec_priv *priv = intf->cb->pd->priv;
	int index, ret;

	index = set_device_index(intf, priv->device_index);
	ret = cros_ec_detect(intf);
	set_device_index(intf, index);

	return ret;
}
