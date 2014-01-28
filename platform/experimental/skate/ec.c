/*
 * Copyright 2014, Google Inc.
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

#define SKATE_EC_BUS		4
#define	SKATE_EC_ADDRESS	0x1e

struct cros_ec_priv skate_ec_priv = {
	.addr.i2c.bus	= SKATE_EC_BUS,		/* may be overridden */
	.addr.i2c.addr	= SKATE_EC_ADDRESS,
};

int skate_ec_setup(struct platform_intf *intf)
{
	int ret;

	MOSYS_CHECK(intf->cb && intf->cb->ec);
	intf->cb->ec->priv = &skate_ec_priv;

	ret = cros_ec_probe_i2c(intf);
	if (ret == 1)
		lprintf(LOG_DEBUG, "CrOS EC found on I2C bus\n");
	else if (ret == 0)
		lprintf(LOG_DEBUG, "CrOS EC not found on I2C bus\n");
	else
		lprintf(LOG_ERR, "Error when probing CrOS EC on I2C bus\n");

	return ret;
}
