/*
 * Copyright (C) 2010 Google Inc.
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

#include <inttypes.h>

#include "drivers/superio.h"
#include "drivers/ite/it8500.h"

#include "mosys/platform.h"

/*
 * mario_pinetrail_ec_name - return EC firmware name string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *mario_pinetrail_ec_name(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;
	uint16_t port;

	if (it8500_get_sioport(intf, &port) <= 0)
		return "Unknown";

	id = get_sio_id(intf, port);
	if (!id)
		return "Unknown";

	return id->name;
}

/*
 * mario_pinetrail_ec_vendor - return EC vendor string
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
static const char *mario_pinetrail_ec_vendor(struct platform_intf *intf)
{
	const struct sio_id *id = NULL;
	uint16_t port;

	if (it8500_get_sioport(intf, &port) <= 0)
		return "Unknown";

	id = get_sio_id(intf, port);
	if (!id)
		return "Unknown";

	return id->vendor;
}

int mario_pinetrail_ec_setup(struct platform_intf *intf)
{
	int rc = 0;

	/* invert logic -- it8500_detect will return 1 if it finds an it8500 EC */
	if (!it8500_detect(intf))
		rc = 1;

	return rc;
}

struct ec_cb mario_pinetrail_ec_cb = {
	.vendor		= mario_pinetrail_ec_vendor,
	.name		= mario_pinetrail_ec_name,
//	.fw_version	= mario_pinetrail_ec_fw_version,
};
