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

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "lib/probe.h"
#include "drivers/sandisk/u100.h"

/* Butterfly SSD -- /dev/sda. */
#define BUTTERFLY_SSD_DEVICE "sda"
#define BUTTERFLY_SSD_DEVICE_PATH "/dev/" BUTTERFLY_SSD_DEVICE

/*
 * butterfly_get_ssd_model_name - Returns SSD model name.
 *
 * @intf:	platform interface.
 *
 * Returns pointer to allocated model name string, or NULL for failure.
 */
static const char *butterfly_get_ssd_model_name(struct platform_intf *intf)
{
	return extract_block_device_model_name(BUTTERFLY_SSD_DEVICE);
}

/*
 * butterfly_get_ssd_phy_speed - Returns SSD PHY speed.
 *
 * @intf:	platform interface.
 *
 * Returns current storage_phy_speed of device, or PHY_SPEED_UNKNOWN in
 * case of error.
 */
static enum storage_phy_speed butterfly_get_ssd_phy_speed(
					struct platform_intf *intf)
{
	char *model_name = (char*)butterfly_get_ssd_model_name(intf);
	enum storage_phy_speed phy_speed = PHY_SPEED_UNKNOWN;

	lprintf(LOG_DEBUG, "%s: SSD model name %s\n", __func__, model_name);
	if (strcmp(model_name, SANDISK_U100_MODEL_NAME) == 0 ||
	    strcmp(model_name, SANDISK_SDSA5GK_MODEL_NAME) == 0)
		phy_speed = sandisk_u100_get_phy_speed(
			BUTTERFLY_SSD_DEVICE_PATH);

	if (model_name != NULL)
		free(model_name);
	return phy_speed;
}

/*
 * butterfly_set_ssd_phy_speed - Sets SSD PHY speed.
 *
 * @intf:	platform interface.
 * @phy_speed:	PHY speed value to set.
 *
 * Returns 0 on success or < 0 on error.
 */
static int butterfly_set_ssd_phy_speed(struct platform_intf *intf,
				       enum storage_phy_speed phy_speed)
{
	char *model_name = (char*)butterfly_get_ssd_model_name(intf);
	int ret = -1;

	lprintf(LOG_DEBUG, "%s: SSD model name %s\n", __func__, model_name);
	if (strcmp(model_name, SANDISK_U100_MODEL_NAME) == 0 ||
	    strcmp(model_name, SANDISK_SDSA5GK_MODEL_NAME) == 0)
		ret = sandisk_u100_set_phy_speed(BUTTERFLY_SSD_DEVICE_PATH,
						 phy_speed);

	if (model_name != NULL)
		free(model_name);
	return ret;
}

struct storage_cb butterfly_storage_cb = {
	.get_model_name	= &butterfly_get_ssd_model_name,
	.get_phy_speed	= &butterfly_get_ssd_phy_speed,
	.set_phy_speed	= &butterfly_set_ssd_phy_speed,
};
