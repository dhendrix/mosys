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
 * gec.c: Generic GEC functions and structures.
 */

#include <inttypes.h>

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/gec.h"

#include "lib/math.h"

int gec_hello(struct platform_intf *intf)
{
	struct gec_params_hello p;
	struct gec_response_hello r;
	int rv;
	struct gec_priv *priv = intf->cb->ec->priv;

	p.in_data = 0xa0b0c0d0;

	rv = priv->cmd(intf, GEC_COMMAND_HELLO, &p,
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

const char *gec_version(struct platform_intf *intf)
{
	static const char *const fw_copies[] = {"unknown", "RO", "A", "B"};
	struct gec_response_get_version r;
	struct gec_response_get_build_info r2;
	const char *ret = NULL;
	struct gec_priv *priv = intf->cb->ec->priv;

	if (priv->cmd(intf, GEC_COMMAND_GET_VERSION,
		      &r, sizeof(r), NULL, 0))
		return NULL;

	if (priv->cmd(intf, GEC_COMMAND_GET_BUILD_INFO,
		      &r2, sizeof(r2), NULL, 0))
		return NULL;

	/* Ensure versions are null-terminated before we print them */
	r.version_string_ro[sizeof(r.version_string_ro) - 1] = '\0';
	r.version_string_rw_a[sizeof(r.version_string_rw_a) - 1] = '\0';
	r.version_string_rw_b[sizeof(r.version_string_rw_b) - 1] = '\0';
	r2.build_string[sizeof(r2.build_string) - 1] = '\0';

	/* Print versions */
	lprintf(LOG_DEBUG, "RO version:    %s\n", r.version_string_ro);
	lprintf(LOG_DEBUG, "RW-A version:  %s\n", r.version_string_rw_a);
	lprintf(LOG_DEBUG, "RW-B version:  %s\n", r.version_string_rw_b);
	lprintf(LOG_DEBUG, "Firmware copy: %s\n",
	       (r.current_image < ARRAY_SIZE(fw_copies) ?
		fw_copies[r.current_image] : "?"));
	lprintf(LOG_DEBUG, "Build info:    %s\n", r2.build_string);

	switch (r.current_image) {
	case GEC_IMAGE_RO:
		ret = mosys_strdup(r.version_string_ro);
		break;
	case GEC_IMAGE_RW_A:
		ret = mosys_strdup(r.version_string_rw_a);
		break;
	case GEC_IMAGE_RW_B:
		ret = mosys_strdup(r.version_string_rw_b);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: cannot determine version\n", __func__);
		break;
	}

	return ret;
}

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int gec_detect(struct platform_intf *intf)
{
	struct gec_params_hello request;
	struct gec_response_hello response;
	int result = 0;
	int ret = 0;
	struct gec_priv *priv;

	if (!intf->cb || !intf->cb->ec || !intf->cb->ec->priv)
		return -1;
	priv = intf->cb->ec->priv;

	/* Say hello to EC. */
	request.in_data = 0xf0e0d0c0;  /* Expect EC will add on 0x01020304. */

	lprintf(LOG_DEBUG, "%s: sending HELLO request with 0x%08x\n",
		__func__, request.in_data);
	result  = priv->cmd(intf, GEC_COMMAND_HELLO,
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
