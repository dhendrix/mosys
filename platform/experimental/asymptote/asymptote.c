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

#include <stdlib.h>
#include <unistd.h>

#include "mosys/command_list.h"
#include "mosys/platform.h"
#include "mosys/intf_list.h"
#include "mosys/log.h"

#include "lib/math.h"
#include "lib/probe.h"

#include "asymptote.h"

static const char *probed_platform_id;

const char *asymptote_tegra2_id_list[] = {
	"Asymptote",
	NULL
};

struct platform_cmd *asymptote_tegra2_sub[] = {
	&cmd_platform,
	NULL
};

int asymptote_tegra2_probe(struct platform_intf *intf)
{
	static int status = 0, probed = 0;
	const char **id;

	if (probed)
		return status;

	for (id = asymptote_tegra2_id_list; id && *id; id++) {
		if (probe_cpuinfo(intf, "Hardware", *id)) {
			status = 1;
			goto asymptote_tegra2_probe_exit;
		}
	}

asymptote_tegra2_probe_exit:
	probed = 1;
	return status;
}

static int asymptote_tegra2_destroy(struct platform_intf *intf)
{
	if (probed_platform_id)
		free((char *)probed_platform_id);
	return 0;
}

struct platform_cb asymptote_tegra2_cb = {
	.sys 		= &asymptote_tegra2_sys_cb,
};

struct platform_intf platform_asymptote_tegra2 = {
	.type		= PLATFORM_ARMV7,
	.name		= "Asymptote",
	.id_list	= asymptote_tegra2_id_list,
	.sub		= asymptote_tegra2_sub,
	.cb		= &asymptote_tegra2_cb,
	.probe		= &asymptote_tegra2_probe,
	.destroy	= &asymptote_tegra2_destroy,
};
