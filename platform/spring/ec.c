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

#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/google/cros_ec.h"

#define SPRING_EC_BUS		4
#define	SPRING_EC_ADDRESS	0x1e

static struct cros_ec_dev dev = {
	.name	= CROS_EC_DEV_NAME,
};

static struct i2c_addr i2c_addr = {
	.bus	= SPRING_EC_BUS,		/* may be overridden */
	.addr	= SPRING_EC_ADDRESS,
};

static struct cros_ec_priv spring_ec_priv = {
	.devfs	= &dev,
	.i2c = &i2c_addr,
};

int spring_ec_setup(struct platform_intf *intf)
{
	MOSYS_CHECK(intf->cb && intf->cb->ec);
	intf->cb->ec->priv = &spring_ec_priv;

	/* Use cros_ec device if available, fall back on raw I2C if needed. */
	if (cros_ec_probe_dev(intf, intf->cb->ec) == 1) {
		lprintf(LOG_DEBUG, "CrOS EC found using /dev interface\n");
		return 1;
	} else {
		lprintf(LOG_DEBUG, "CrOS EC not found with cros_ec driver\n");
	}

	if (cros_ec_probe_i2c(intf) == 1) {
		lprintf(LOG_DEBUG, "CrOS EC found using raw I2C interface\n");
		return 1;
	} else {
		lprintf(LOG_DEBUG, "CrOS EC not found using raw I2C\n");
	}

	return 0;
}
