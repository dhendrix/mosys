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
 * ich_generic.c: Common Intel ICH functions.
 */

#include <inttypes.h>

#include "mosys/platform.h"

#include "drivers/intel/ich_generic.h"
#include "intf/pci.h"
#include "intf/mmio.h"
#include "lib/math.h"

#define ICH_GCS_OFFSET		0x3410

static int ich_get_rcba_addr(struct platform_intf *intf, uint32_t *val)
{
	if (pci_read32(intf, 0x00, 0x1f, 0x00, 0xf0, val) < 0)
		return -1;

	*val &= ~__mask(13, 0);

	return 0;
}

static int ich_get_gcs_addr(struct platform_intf *intf, uint32_t *val)
{
	uint32_t rcba_addr = 0;

	if (ich_get_rcba_addr(intf, &rcba_addr) < 0)
		return -1;

	*val = rcba_addr + ICH_GCS_OFFSET;

	return 0;
}

int ich_get_bbs(struct platform_intf *intf)
{
	uint32_t gcs_addr = 0, gcs_val = 0;

	if (ich_get_gcs_addr(intf, &gcs_addr) < 0)
		return -1;

	if (mmio_read32(intf, gcs_addr, &gcs_val) < 0)
		return -1;

	return (gcs_val >> 10) & 0xff;
}

int ich_set_bbs(struct platform_intf *intf, int bbs)
{
	uint32_t gcs_addr = 0, gcs_val = 0;

	if (bbs < 0 || bbs > 3)
		return -1;

	if (ich_get_gcs_addr(intf, &gcs_addr) < 0)
		return -1;

	if (mmio_read32(intf, gcs_addr, &gcs_val) < 0)
		return -1;

	gcs_val = (gcs_val & ~__mask(11, 10)) | (bbs << 10);

	if (mmio_write32(intf, gcs_addr, gcs_val) < 0)
		return -1;

	return 0;
}
