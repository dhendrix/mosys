/*
 * Copyright (C) 2011 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
