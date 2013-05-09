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
 *
 * lpss_generic.c: Common Intel LPSS functions
 */

#include <inttypes.h>
#include <unistd.h>

#include "mosys/platform.h"

#include "drivers/intel/ich_generic.h"
#include "drivers/intel/lpss_generic.h"

#include "intf/io.h"
#include "intf/pci.h"
#include "intf/mmio.h"

#include "lib/math.h"

#define LPSS_GCS_OFFSET		0x3410

static int lpss_get_rcba_addr(struct platform_intf *intf, uint32_t *val)
{
	if (pci_read32(intf, 0x00, 0x1f, 0x00, 0xf0, val) < 0)
		return -1;

	*val &= ~__mask(13, 0);

	return 0;
}

static int lpss_get_gcs_addr(struct platform_intf *intf, uint32_t *val)
{
	uint32_t rcba_addr = 0;

	if (lpss_get_rcba_addr(intf, &rcba_addr) < 0)
		return -1;

	*val = rcba_addr + LPSS_GCS_OFFSET;

	return 0;
}

int lpss_get_bbs(struct platform_intf *intf)
{
	uint32_t gcs_addr = 0, gcs_val = 0;

	if (lpss_get_gcs_addr(intf, &gcs_addr) < 0)
		return -1;

	if (mmio_read32(intf, gcs_addr, &gcs_val) < 0)
		return -1;

	return (gcs_val >> 10) & 1;
}

int lpss_set_bbs(struct platform_intf *intf, int bbs)
{
	uint32_t gcs_addr = 0, gcs_val = 0;

	switch (bbs) {
	case LPSS_BBS_LPC:
	case LPSS_BBS_SPI:
		break;
	default:
		return -1;
	}

	if (lpss_get_gcs_addr(intf, &gcs_addr) < 0)
		return -1;

	if (mmio_read32(intf, gcs_addr, &gcs_val) < 0)
		return -1;

	gcs_val = (gcs_val & ~(1ULL << 10)) | ((bbs & 1) << 10);

	if (mmio_write32(intf, gcs_addr, gcs_val) < 0)
		return -1;

	return 0;
}
