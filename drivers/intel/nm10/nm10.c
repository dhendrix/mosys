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
 *
 * nm10.c: NM10 helper functions
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "intf/mmio.h"
#include "intf/pci.h"

#include "lib/math.h"

#include "drivers/intel/nm10.h"

#define NM10_GCS_OFFSET		0x3410

static int nm10_get_rcba_addr(struct platform_intf *intf, uint32_t *val)
{
	if (pci_read32(intf, 0x00, 0x1f, 0x00, 0xf0, val) < 0)
		return -1;

	*val &= ~__mask(13, 0);

	return 0;
}

static int nm10_get_gcs_addr(struct platform_intf *intf, uint32_t *val)
{
	uint32_t rcba_addr = 0;

	if (nm10_get_rcba_addr(intf, &rcba_addr) < 0)
		return -1;

	*val = rcba_addr + NM10_GCS_OFFSET;

	return 0;
}

enum ich_bbs nm10_get_bbs(struct platform_intf *intf)
{
	uint32_t gcs_addr = 0, gcs_val = 0;

	if (nm10_get_gcs_addr(intf, &gcs_addr) < 0)
		return -1;

	if (mmio_read32(intf, gcs_addr, &gcs_val) < 0)
		return -1;

	return (gcs_val >> 10) & 0xff;
}

int nm10_set_bbs(struct platform_intf *intf, enum ich_bbs bbs)
{
	uint32_t gcs_addr = 0, gcs_val = 0;

	if (bbs == ICH_BBS_UNKNOWN)
		return -1;

	if (nm10_get_gcs_addr(intf, &gcs_addr) < 0)
		return -1;

	if (mmio_read32(intf, gcs_addr, &gcs_val) < 0)
		return -1;

	gcs_val = (gcs_val & ~__mask(11, 10)) | (bbs << 10);

	if (mmio_write32(intf, gcs_addr, gcs_val) < 0)
		return -1;

	return 0;
}
