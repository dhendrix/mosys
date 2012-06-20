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

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/smbios.h"
#include "lib/vpd.h"

#include "parrot.h"

int parrot_vpd_setup(struct platform_intf *intf)
{
	unsigned int rom_base, rom_size;

	/* FIXME: SMBIOS might not be useful for ROM size detection here since
	   BIOS size will only reflect the size of the BIOS flash partition */
	rom_size = PARROT_HOST_FIRMWARE_ROM_SIZE;
	rom_base = 0xffffffff - rom_size + 1;
	vpd_rom_base = rom_base;
	vpd_rom_size = rom_size;

	return 0;
}

static char *parrot_vpd_get_serial(struct platform_intf *intf)
{
	if (intf->cb->vpd && intf->cb->vpd->system_serial)
		return intf->cb->vpd->system_serial(intf);
	else
		return NULL;

}

static char *parrot_vpd_get_sku(struct platform_intf *intf)
{
	if (intf->cb->vpd && intf->cb->vpd->system_sku)
		return intf->cb->vpd->system_sku(intf);
	else
		return NULL;
}


static char *parrot_vpd_get_google_hwqualid(struct platform_intf *intf)
{
	/* FIXME: noop for now */
	return NULL;
}

struct vpd_cb parrot_vpd_cb = {
	.system_serial		= &parrot_vpd_get_serial,
	.system_sku		= &parrot_vpd_get_sku,
	.google_hwqualid	= &parrot_vpd_get_google_hwqualid,
};
