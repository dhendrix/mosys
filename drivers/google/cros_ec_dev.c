/*
 * Copyright 2013 Google Inc.
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
 * cros_ec_dev.c: Chrome EC interface via /dev
 */

#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/cros_ec.h"
#include "drivers/google/cros_ec_dev.h"
#include "drivers/google/cros_ec_commands.h"

#include "lib/file.h"
#include "lib/math.h"

#define CROS_EC_COMMAND_RETRIES	50

/* ec device interface v1 (used with Chrome OS v3.18 and earlier) */

/*
 * Wait for a command to complete, then return the response
 *
 * This is called when we get an EAGAIN response from the EC. We need to
 * send EC_CMD_GET_COMMS_STATUS commands until the EC indicates it is
 * finished the command that we originally sent.
 *
 * returns 0 if command is successful, <0 to indicate timeout or error
 */
static int command_wait_for_response(struct cros_ec_priv *priv)
{
	struct ec_response_get_comms_status status;
	struct cros_ec_command cmd;
	int ret;
	int i;

	cmd.version = 0;
	cmd.command = EC_CMD_GET_COMMS_STATUS;
	cmd.outdata = NULL;
	cmd.outsize = 0;
	cmd.indata = (uint8_t *)&status;
	cmd.insize = sizeof(status);

	for (i = 1; i <= CROS_EC_COMMAND_RETRIES; i++) {
		ret = ioctl(priv->devfs->fd, CROS_EC_DEV_IOCXCMD, &cmd,
			    sizeof(cmd));
		if (ret) {
			lprintf(LOG_ERR, "%s: CrOS EC command failed: %d\n",
				 __func__, ret);
			ret = -EC_RES_ERROR;
			break;
		}

		if (!(status.flags & EC_COMMS_STATUS_PROCESSING)) {
			ret = -EC_RES_SUCCESS;
			break;
		}

		usleep(1000);
	}

	return ret;
}

/*
 * cros_ec_command_dev - Issue command to CROS_EC device
 *
 * @command:	command code
 * @outdata:	data to send to EC
 * @outsize:	number of bytes in outbound payload
 * @indata:	(unallocated) buffer to store data received from EC
 * @insize:	number of bytes in inbound payload
 *
 * This uses the kernel Chrome OS EC driver to communicate with the EC.
 *
 * The outdata and indata buffers contain payload data (if any); command
 * and response codes as well as checksum data are handled transparently by
 * this function.
 *
 * Returns >=0 for success, or negative if other error.
 */
static int cros_ec_command_dev(struct platform_intf *intf, struct ec_cb *ec,
		int command, int version, const void *indata, int insize,
		const void *outdata, int outsize)
{
	struct cros_ec_priv *priv;
	struct cros_ec_command cmd;
	int ret;

	MOSYS_DCHECK(ec && ec->priv);
	priv = ec->priv;

	cmd.version = version;
	cmd.command = command;
	cmd.outdata = outdata;
	cmd.outsize = outsize;
	cmd.indata = (uint8_t *)indata;
	cmd.insize = insize;
	ret = ioctl(priv->devfs->fd, CROS_EC_DEV_IOCXCMD, &cmd, sizeof(cmd));
	if (ret < 0 && errno == -EAGAIN)
		ret = command_wait_for_response(priv);

	if (ret < 0) {
		lprintf(LOG_ERR, "%s: Transfer failed: %d\n", __func__, ret);
		return -EC_RES_ERROR;
	}

	return 0; /* Should we return ret here? */
}

/*
 * ec device interface v2
 * (used with upstream kernel as well as with Chrome OS v4.4 and later)
 */

static int command_wait_for_response_v2(struct cros_ec_priv *priv)
{
	uint8_t s_cmd_buf[sizeof(struct cros_ec_command_v2) +
			  sizeof(struct ec_response_get_comms_status)];
	struct ec_response_get_comms_status *status;
	struct cros_ec_command_v2 *s_cmd;
	int ret;
	int i;

	s_cmd = (struct cros_ec_command_v2 *)s_cmd_buf;
	status = (struct ec_response_get_comms_status *)s_cmd->data;

	s_cmd->version = 0;
	s_cmd->command = EC_CMD_GET_COMMS_STATUS;
	s_cmd->outsize = 0;
	s_cmd->insize = sizeof(*status);

	for (i = 1; i <= CROS_EC_COMMAND_RETRIES; i++) {
		ret = ioctl(priv->devfs->fd, CROS_EC_DEV_IOCXCMD_V2, s_cmd_buf,
			    sizeof(s_cmd_buf));
		if (ret) {
			lprintf(LOG_ERR, "%s: CrOS EC command failed: %d\n",
				 __func__, ret);
			ret = -EC_RES_ERROR;
			break;
		}

		if (!(status->flags & EC_COMMS_STATUS_PROCESSING)) {
			ret = -EC_RES_SUCCESS;
			break;
		}

		usleep(1000);
	}

	return ret;
}

static int cros_ec_command_dev_v2(struct platform_intf *intf, struct ec_cb *ec,
		int command, int version, const void *indata, int insize,
		const void *outdata, int outsize)
{
	struct cros_ec_priv *priv;
	struct cros_ec_command_v2 *s_cmd;
	int size = sizeof(struct cros_ec_command_v2) + __max(outsize, insize);
	int ret;

	MOSYS_DCHECK(ec && ec->priv);
	priv = ec->priv;

	s_cmd = mosys_malloc(size);
	s_cmd->command = command;
	s_cmd->version = version;
	s_cmd->result = 0xff;
	s_cmd->outsize = outsize;
	s_cmd->insize = insize;
	if (outdata)
		memcpy(s_cmd->data, outdata, outsize);

	ret = ioctl(priv->devfs->fd, CROS_EC_DEV_IOCXCMD_V2, s_cmd, size);
	if (ret < 0 && errno == -EAGAIN)
		ret = command_wait_for_response_v2(priv);
	if (ret < 0) {
		lprintf(LOG_ERR, "%s: Transfer failed: %d\n", __func__, ret);
		free(s_cmd);
		return -EC_RES_ERROR;
	}

	/*
	 * The function parameter declares indata as pointer to a constant char,
	 * which does not make much sense as its content is expected to be
	 * overwritten by this function. Use a typecast for now to avoid the
	 * inevitable warning.
	 */
	if (indata)
		memcpy((void *)indata, s_cmd->data, __min(ret, insize));
	free(s_cmd);
	return 0;
}

/*
 * Attempt to communicate with kernel using old ioctl format.
 * If it returns ENOTTY, assume that this kernel uses the new format.
 */
static int ec_dev_is_v2(int fd)
{
	struct ec_params_hello h_req = {
		.in_data = 0xa0b0c0d0
	};
	struct ec_response_hello h_resp;
	struct cros_ec_command s_cmd = { };
	int r;

	s_cmd.command = EC_CMD_HELLO;
	s_cmd.result = 0xff;
	s_cmd.outsize = sizeof(h_req);
	s_cmd.outdata = (uint8_t *)&h_req;
	s_cmd.insize = sizeof(h_resp);
	s_cmd.indata = (uint8_t *)&h_resp;

	r = ioctl(fd, CROS_EC_DEV_IOCXCMD, &s_cmd, sizeof(s_cmd));
	if (r < 0 && errno == ENOTTY)
		return 1;

	return 0;
}

static int cros_ec_close_dev(struct platform_intf *intf, struct ec_cb *ec)
{
	struct cros_ec_priv *priv;
	priv = ec->priv;
	return close(priv->devfs->fd);
}

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_ec_probe_dev(struct platform_intf *intf, struct ec_cb *ec)
{
	int ret = 0;
	struct cros_ec_priv *priv;
	char filename[PATH_MAX];

	MOSYS_DCHECK(ec && ec->priv);
	priv = ec->priv;

	sprintf(filename, "%s/%s", mosys_get_root_prefix(), priv->devfs->name);
	priv->devfs->fd = open(filename, O_RDWR);
	if (priv->devfs->fd < 0) {
		lprintf(LOG_DEBUG, "%s: unable to open \"%s\"\n",
				__func__, filename);
		ret = -1;
	} else {
		ec->destroy = cros_ec_close_dev;
		if (ec_dev_is_v2(priv->devfs->fd))
			priv->cmd = cros_ec_command_dev_v2;
		else
			priv->cmd = cros_ec_command_dev;
		ret = cros_ec_detect(intf, ec);
	}

	return ret;
}
