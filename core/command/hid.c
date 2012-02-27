/*
 * Copyright 2012, Google Inc.
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

#include <errno.h>

#include "mosys/kv_pair.h"
#include "mosys/platform.h"

static int touchpad_all_info_cmd(struct platform_intf *intf,
                                 struct platform_cmd *cmd, int argc,
                                 char **argv)
{
	struct kv_pair *kv;

	/* put ec vendor, name, and firmware version in kv=pair format */
	if (!intf->cb->hid ||
	    !intf->cb->hid->tp ||
	    !intf->cb->hid->tp->vendor ||
	    !intf->cb->hid->tp->name ||
	    !intf->cb->hid->tp->fw_version ||
	    !intf->cb->hid->tp->hw_version) {
		return -ENOSYS;
	}

	kv = kv_pair_new();

	kv_pair_add(kv, "vendor", intf->cb->hid->tp->vendor(intf));
	kv_pair_add(kv, "name", intf->cb->hid->tp->name(intf));
	kv_pair_add(kv, "fw_version", intf->cb->hid->tp->fw_version(intf));
	kv_pair_add(kv, "hw_version", intf->cb->hid->tp->hw_version(intf));

	kv_pair_print(kv);
	kv_pair_free(kv);
	return 0;

}

static int touchpad_fw_version_cmd(struct platform_intf *intf,
                                   struct platform_cmd *cmd, int argc,
                                   char **argv)
{
	struct kv_pair *kv;

	/* put ec vendor, name, and firmware version in kv=pair format */
	if (!intf->cb->hid ||
	    !intf->cb->hid->tp ||
	    !intf->cb->hid->tp->fw_version) {
		return -ENOSYS;
	}

	kv = kv_pair_new();

	kv_pair_add(kv, "fw_version", intf->cb->hid->tp->fw_version(intf));

	kv_pair_print(kv);
	kv_pair_free(kv);
	return 0;

}

static int touchpad_id_cmd(struct platform_intf *intf,
                           struct platform_cmd *cmd, int argc, char **argv)
{
	struct kv_pair *kv;

	/* put ec vendor, name, and firmware version in kv=pair format */
	if (!intf->cb->hid ||
	    !intf->cb->hid->tp ||
	    !intf->cb->hid->tp->vendor ||
	    !intf->cb->hid->tp->name ||
	    !intf->cb->hid->tp->hw_version) {
		return -ENOSYS;
	}

	kv = kv_pair_new();

	kv_pair_add(kv, "vendor", intf->cb->hid->tp->vendor(intf));
	kv_pair_add(kv, "name", intf->cb->hid->tp->name(intf));
	kv_pair_add(kv, "hw_version", intf->cb->hid->tp->hw_version(intf));

	kv_pair_print(kv);
	kv_pair_free(kv);
	return 0;

}

struct platform_cmd hid_touchpad_cmds[] = {
	{
		.name	= "all",
		.desc	= "Print all available touchpad info",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = touchpad_all_info_cmd }
	},
	{
		.name	= "fw_version",
		.desc	= "Print touchpad firmware version",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = touchpad_fw_version_cmd }
	},
	{
		.name	= "id",
		.desc	= "Print touchpad id info",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = touchpad_id_cmd }
	},
	{ NULL }
};

struct platform_cmd hid_cmds[] = {
	{
		.name	= "tp",
		.desc	= "Information about touchpad",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = hid_touchpad_cmds }
	},
	{ NULL }
};

struct platform_cmd cmd_hid = {
	.name	= "hid",
	.desc	= "Human Interface Device Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = hid_cmds }
};
