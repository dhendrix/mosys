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

#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/cros_ec.h"
#include "drivers/google/cros_ec_dev.h"
#include "drivers/google/cros_ec_commands.h"

#include "lib/file.h"

#define CROS_EC_DEV_NAME		"/dev/cros_ec"
#define CROS_EC_COMMAND_RETRIES	50

static int cros_ec_fd;		/* File descriptor of CROS_EC_DEV_NAME */

/*
 * Wait for a command to complete, then return the response
 *
 * This is called when we get an EAGAIN response from the EC. We need to
 * send EC_CMD_GET_COMMS_STATUS commands until the EC indicates it is
 * finished the command that we originally sent.
 *
 * returns 0 if command is successful, <0 to indicate timeout or error
 */
static int command_wait_for_response(void)
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
		ret = ioctl(cros_ec_fd, CROS_EC_DEV_IOCXCMD, &cmd, sizeof(cmd));
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
static int cros_ec_command_dev(struct platform_intf *intf, int command,
			   int version, const void *indata, int insize,
			   const void *outdata, int outsize)
{
	struct cros_ec_command cmd;
	int ret;

	cmd.version = version;
	cmd.command = command;
	cmd.outdata = outdata;
	cmd.outsize = outsize;
	cmd.indata = (uint8_t *)indata;
	cmd.insize = insize;
	ret = ioctl(cros_ec_fd, CROS_EC_DEV_IOCXCMD, &cmd, sizeof(cmd));
	if (ret < 0 && errno == -EAGAIN)
		ret = command_wait_for_response();

	if (ret < 0) {
		lprintf(LOG_ERR, "%s: Transfer failed: %d\n", __func__, ret);
		return -EC_RES_ERROR;
	}

	return 0; /* Should we return ret here? */
}

void cros_ec_close_dev(void)
{
	close(cros_ec_fd);
}

/* returns 1 if EC detected, 0 if not, <0 to indicate failure */
int cros_ec_probe_dev(struct platform_intf *intf)
{
	int ret = 0;
	struct cros_ec_priv *priv;
	char filename[PATH_MAX];

	MOSYS_DCHECK(intf->cb->ec && intf->cb->ec->priv);
	priv = intf->cb->ec->priv;

	sprintf(filename, "%s/%s", mosys_get_root_prefix(), CROS_EC_DEV_NAME);
	cros_ec_fd = open(filename, O_RDWR);
	if (cros_ec_fd < 0) {
		lprintf(LOG_DEBUG, "%s: unable to open \"%s\"\n",
				__func__, filename);
		ret = -1;
	} else {
		intf->cb->ec->destroy = cros_ec_close_dev;
		priv->cmd = cros_ec_command_dev;
		ret = 1;
	}

	return ret;
}
