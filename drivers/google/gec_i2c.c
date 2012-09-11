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
 * gec_i2c.c: Subset of Google I2C EC interface functionality (ported from
 * chromium os repo)
 */

#include <inttypes.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/output.h"
#include "mosys/platform.h"

#include "drivers/google/gec.h"
#include "drivers/google/gec_ec_commands.h"

#include "lib/file.h"
#include "lib/math.h"

#include "intf/i2c.h"

#define SYSFS_I2C_DEV_ROOT	"/sys/bus/i2c/devices"
#define GEC_I2C_ADAPTER_NAME	"cros_ec_i2c"

/* protocol bytes (command/response code + checksum byte) */
#define GEC_PROTO_BYTES		2

/* Sends a command to the EC.  Returns the command status code, or
 * -1 if other error. */
static int gec_command_i2c(struct platform_intf *intf,
			   int command, int command_version,
			   const void *indata, int insize,
			   const void *outdata, int outsize) {
	int ret = -1;
	uint8_t *req_buf = NULL, *resp_buf = NULL;
	int req_len = 0, resp_len = 0;
	int i, len, csum;
	struct gec_priv *priv = intf->cb->ec->priv;
	struct i2c_addr *addr = &(priv->addr.i2c);

	if (insize > EC_HOST_PARAM_SIZE || outsize > EC_HOST_PARAM_SIZE) {
		lprintf(LOG_DEBUG, "%s: data size too big\n", __func__);
		goto done;
	}

	if (command_version) {
		/* New-style command */
		req_len = 4 + outsize;
		req_buf = mosys_calloc(1, req_len);
		req_buf[0] = EC_CMD_VERSION0 + command_version;
		req_buf[1] = command;
		req_buf[2] = outsize;
		memcpy(&req_buf[3], outdata, outsize);
		req_buf[req_len - 1] = rolling8_csum(req_buf, req_len - 1);

		resp_len = 3 + insize;
		resp_buf = mosys_calloc(1, resp_len);
		if (!resp_buf)
			goto done;
	} else {
		/* Old-style command */
		if (outsize) {
			req_len = outsize + GEC_PROTO_BYTES;
			req_buf = mosys_calloc(1, req_len);

			/* copy message payload and compute checksum */
			memcpy(&req_buf[1], outdata, outsize);
			req_buf[req_len - 1] = rolling8_csum(outdata, outsize);
		} else {
			/* request buffer will hold command code only */
			req_len = 1;
			req_buf = mosys_calloc(1, req_len);
		}
		req_buf[0] = command;

		if (insize) {
			resp_len = insize + GEC_PROTO_BYTES;
			resp_buf = mosys_calloc(1, resp_len);
			if (!resp_buf)
				goto done;
		} else {
			/* response buffer will hold error code only */
			resp_len = 1;
			resp_buf = mosys_calloc(1, resp_len);
			if (!resp_buf)
				goto done;
		}
	}

	if (mosys_get_verbosity() == LOG_SPEW) {
		lprintf(LOG_SPEW, "%s: dumping req_buf\n", __func__);
		print_buffer(req_buf, req_len);
	}

	ret = intf->op->i2c->i2c_transfer(intf,
					  addr->bus, addr->addr,
					  req_buf, req_len,
					  resp_buf, resp_len);
	if (ret)
		goto done;

	if (mosys_get_verbosity() == LOG_SPEW) {
		lprintf(LOG_SPEW, "%s: dumping resp_buf\n", __func__);
		print_buffer(resp_buf, resp_len);
	}

	if (command_version) {
		/* New-style command */
		ret = resp_buf[0];
		if (ret) {
			lprintf(LOG_DEBUG,
				"command 0x%02x returned an error %d\n",
				command, ret);
			goto done;
		}

		len = resp_buf[1];
		if (len != insize) {
			lprintf(LOG_DEBUG,
				"bad response payload size (got %d from EC,"
				"expected %d)\n",
				len, insize);
			ret = -1;
			goto done;
		}

		csum = rolling8_csum(resp_buf, 2 + len);
		if (csum != resp_buf[resp_len - 1]) {
			lprintf(LOG_DEBUG,
				"bad checksum (got 0x%02x from EC,"
				"calculated 0x%02x)\n",
				resp_buf[resp_len - 1], csum);
			ret = -1;
			goto done;
		}

		if (insize)
			memcpy((void *)indata, &resp_buf[2], insize);
	} else {
		/* Old-style command */
		/* check response error code */
		ret = resp_buf[0];
		if (ret) {
			lprintf(LOG_DEBUG,
				"command 0x%02x returned an error %d\n",
				command, resp_buf[0]);
		}

		if (insize) {
			/* copy response packet payload and compute checksum */
			for (i = 0, csum = 0; i < insize; i++)
				csum += resp_buf[i + 1];
			csum &= 0xff;

			if (csum != resp_buf[resp_len - 1]) {
				lprintf(LOG_DEBUG,
					"bad checksum (got 0x%02x from EC,"
					"calculated 0x%02x\n",
					resp_buf[resp_len - 1], csum);
				ret = -1;
				goto done;
			}

			memcpy((void *)indata, &resp_buf[1], insize);
		}
	}
done:
	if (resp_buf)
		free(resp_buf);
	if (req_buf)
		free(req_buf);
	return ret;
}

/* returns bus number if found, <0 otherwise */
static int gec_probe_i2c_sysfs(struct platform_intf *intf)
{
	const char *path, *s;
	int ret = -1, bus;
	struct ll_node *list = NULL, *head;

	lprintf(LOG_DEBUG, "%s: probing for GEC on I2C...\n", __func__);

	/*
	 * It is possible for the MFD to show up on a different bus than
	 * its I2C adapter. For ChromeOS EC, the I2C passthru adapter piggy-
	 * backs on top of the kernel EC driver. This allows the kernel to
	 * use the MFD while allowing userspace access to the I2C module.
	 *
	 * So for the purposes of finding the correct bus, use the name of
	 * the I2C adapter and not the MFD itself.
	 */
	list = scanft(&list, SYSFS_I2C_DEV_ROOT,
		      "name", GEC_I2C_ADAPTER_NAME, 1);
	if (!list_count(list)) {
		lprintf(LOG_DEBUG, "GEC I2C adapter not found\n");
		goto gec_probe_sysfs_exit;
	}

	head = list_head(list);
	path = (char *)head->data;

	/*
	 * i2c-* may show up more than once in the path (especially in the
	 * case of the MFD with passthru I2C adapter), so use whichever
	 * instance shows up last.
	 */
	for (s = path + strlen(path) - 4; s > path; s--) {
		if (!strncmp(s, "i2c-", 4))
			break;
	}

	if ((s == path) || (sscanf(s, "i2c-%u", &bus) != 1)) {
		lprintf(LOG_ERR, "Unable to parse I2C bus number\n");
		goto gec_probe_sysfs_exit;
	}

	if ((bus >= 0) && (bus <= 255))
		ret = bus;

gec_probe_sysfs_exit:
	list_cleanup(&list);
	return ret;
}

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int gec_probe_i2c(struct platform_intf *intf)
{
	int ret = -1, bus;
	struct gec_priv *priv;

	MOSYS_DCHECK(intf->cb->ec && intf->cb->ec->priv);
	priv = intf->cb->ec->priv;

	bus = gec_probe_i2c_sysfs(intf);
	if (bus >= 0) {
		lprintf(LOG_DEBUG, "%s: Overriding bus %d with %d\n",
			__func__, priv->addr.i2c.bus, bus);
		priv->addr.i2c.bus = bus;
	}

	priv->cmd = gec_command_i2c;
	ret = gec_detect(intf);
	if (ret == 1)
		lprintf(LOG_DEBUG, "GEC detected on I2C bus\n");

	return ret;
}
