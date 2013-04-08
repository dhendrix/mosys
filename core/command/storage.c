/*
 * Copyright 2013, Google Inc.
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
#include <stdio.h>

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "mosys/alloc.h"

/*
 * storage_get_model_name_cmd - Prints model name of storage device.
 *
 * Returns 0 on success < 0 on failure.
 */
static int storage_get_model_name_cmd(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	const char *model_name;
	struct kv_pair *kv;

	if (!intf->cb->storage || !intf->cb->storage->get_model_name) {
		errno = ENOSYS;
		return -1;
	}

	model_name = intf->cb->storage->get_model_name(intf);
	if (!model_name) {
		errno = EIO;
		return -1;
	}

	kv = kv_pair_new();
	kv_pair_add(kv, "model_name", model_name);
	kv_pair_print(kv);
	kv_pair_free(kv);
	free((char*)model_name);

	return 0;
}

/*
 * storage_get_phy_speed_cmd - Prints PHY speed of storage device.
 *
 * Returns 0 on success < 0 on failure.
 */
static int storage_get_phy_speed_cmd(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	char phy_speed_str[8];
	enum storage_phy_speed phy_speed;
	struct kv_pair *kv;

	if (!intf->cb->storage || !intf->cb->storage->get_phy_speed) {
		errno = ENOSYS;
		return -1;
	}

	phy_speed = intf->cb->storage->get_phy_speed(intf);

	if (phy_speed == PHY_SPEED_UNKNOWN) {
		errno = EIO;
		sprintf(phy_speed_str, "unknown");
	}
	else
		sprintf(phy_speed_str, "SATA%d", phy_speed);

	kv = kv_pair_new();
	kv_pair_add(kv, "phy_speed", phy_speed_str);
	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}


/*
 * storage_set_phy_speed_cmd - Sets PHY speed of storage device.
 *
 * Argument:	"SATAX", where X is [1,3] (SATA protocol revision).
 * Returns 0 on success < 0 on failure.
 */
static int storage_set_phy_speed_cmd(struct platform_intf *intf,
		struct platform_cmd *cmd, int argc, char **argv)
{
	enum storage_phy_speed phy_speed;
	char *phy = NULL;
	int ret;

	if (argc == 1)
		phy = argv[0];
	else {
		errno = EINVAL;
		return -1;
	}

	if (phy != NULL && strlen(phy) == 5 && strncmp(phy, "SATA", 4) == 0)
		phy_speed = strtoul(&phy[4], NULL, 10);
	else
		phy_speed = PHY_SPEED_UNKNOWN;

	if (phy_speed < PHY_SPEED_SATA1 || phy_speed > PHY_SPEED_SATA3) {
		errno = EINVAL;
		return -1;
	}

	if (!intf->cb->storage || !intf->cb->storage->set_phy_speed) {
		errno = ENOSYS;
		return -1;
	}

	ret = intf->cb->storage->set_phy_speed(intf, phy_speed);
	/* Don't return error if no supported is found. */
	if (ret == -ENOTSUP)
		return 0;
	else
		return ret;
}

struct platform_cmd storage_print_cmds[] = {
	{
		.name	= "model",
		.desc	= "Print Storage Model Name",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = storage_get_model_name_cmd },
	},
	{
		.name	= "phy_speed",
		.desc	= "Print Storage PHY Speed",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = storage_get_phy_speed_cmd },
	},
	{ NULL }
};

struct platform_cmd storage_set_cmds[] = {
	{
		.name	= "phy_speed",
		.desc	= "Set Storage Device PHY Speed",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = storage_set_phy_speed_cmd },
	},
	{ NULL }
};

struct platform_cmd storage_cmds[] = {
	{
		.name	= "print",
		.desc	= "Print Storage Info",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = storage_print_cmds },
	},
	{
		.name	= "set",
		.desc	= "Set Storage Parameters",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = storage_set_cmds },
	},
	{ NULL }
};

struct platform_cmd cmd_storage = {
	.name	= "storage",
	.desc	= "Storage Device Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = storage_cmds }
};
