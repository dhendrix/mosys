/*
 * Copyright 2017, Google Inc.
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

#include <stdio.h>
#include <errno.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

static int fp_info(struct platform_intf *intf,
                   struct platform_cmd *cmd, int argc, char **argv)
{
	struct kv_pair *kv;
	int rc;

	/* put FP vendor, name, and firmware version in kv=pair format */
	if (!intf->cb->fp ||
	    !intf->cb->fp->vendor ||
	    !intf->cb->fp->name ||
	    !intf->cb->fp->fw_version) {
		errno = ENOSYS;
		return -1;
	}

	kv = kv_pair_new();

	kv_pair_add(kv, "vendor", intf->cb->fp->vendor(intf, intf->cb->fp));
	kv_pair_add(kv, "name", intf->cb->fp->name(intf, intf->cb->fp));
	kv_pair_add(kv, "fw_version",
		    intf->cb->fp->fw_version(intf, intf->cb->fp));

	rc = kv_pair_print(kv);
	kv_pair_free(kv);
	return rc;
}

struct platform_cmd fp_cmds[] = {
	{
		.name	= "info",
		.desc	= "Print basic FP MCU information",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = fp_info}
	},
	/* TODO: add a sub-menu for FP commands */
	{ NULL }
};

struct platform_cmd cmd_fp = {
	.name	= "fp",
	.desc	= "fingerprint MCU information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = fp_cmds }
};
