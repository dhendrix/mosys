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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "mosys/platform.h"

#include "mosys/log.h"
#include "mosys/kv_pair.h"

static int print_platforminfo(const char *key, const char *value);

static int platform_vendor_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sys || !intf->cb->sys->vendor) {
		errno = ENOSYS;
		return -1;
	}

	return print_platforminfo("vendor", intf->cb->sys->vendor(intf));
}

static int platform_name_cmd(struct platform_intf *intf,
                             struct platform_cmd *cmd,
                             int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sys || !intf->cb->sys->name) {
		errno = ENOSYS;
		return -1;
	}

	return print_platforminfo("name", intf->cb->sys->name(intf));
}

static int platform_version_cmd(struct platform_intf *intf,
			      struct platform_cmd *cmd,
			      int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sys || !intf->cb->sys->version) {
		errno = ENOSYS;
		return -1;
	}

	return print_platforminfo("version", intf->cb->sys->version(intf));
}

static int platform_family_cmd(struct platform_intf *intf,
                               struct platform_cmd *cmd,
                               int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sys || !intf->cb->sys->family) {
		errno = ENOSYS;
		return -1;
	}

	return print_platforminfo("family", intf->cb->sys->family(intf));
}

static int platform_model_cmd(struct platform_intf *intf,
			      struct platform_cmd *cmd,
			      int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sys || !intf->cb->sys->model) {
		errno = ENOSYS;
		return -1;
	}

	return print_platforminfo("model", intf->cb->sys->model(intf));
}

static int print_platforminfo(const char *key, const char *value)
{
	struct kv_pair *kv = kv_pair_new();
	int rc;

	if (!value) {
		lprintf(LOG_ERR, "Unable to determine platform %s\n", key);
		return -1;
	}

	kv_pair_add(kv, key, value);
	rc = kv_pair_print(kv);
	kv_pair_free(kv);
	free((char*)value);
	return rc;
}

static int platform_reset_cmd(struct platform_intf *intf,
                              struct platform_cmd *cmd,
                              int argc, char **argv)
{
	if (!intf->cb || !intf->cb->sys || !intf->cb->sys->reset) {
		errno = ENOSYS;
		return -1;
	}

	/* return in case reset function fails somehow */
	return intf->cb->sys->reset(intf);
}

struct platform_cmd platform_cmds[] = {
	{
		.name	= "vendor",
		.desc	= "Display Platform Vendor",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_vendor_cmd }
	},
	{
		.name	= "name",
		.desc	= "Display Platform Product Name",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_name_cmd }
	},
	{
		.name	= "model",
		.desc	= "Display Model",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_model_cmd }
	},
	{
		.name	= "version",
		.desc	= "Display Platform Version",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_version_cmd }
	},
	{
		.name	= "family",
		.desc	= "Display Platform Family",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = platform_family_cmd }
	},
	{
		.name	= "reset",
		.desc	= "(Hard-)Reset Platform",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = platform_reset_cmd }
	},
	{NULL}
};

struct platform_cmd cmd_platform = {
	.name	= "platform",
	.desc	= "Platform Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = platform_cmds }
};
