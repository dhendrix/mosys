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

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/mmio.h"

#include "lib/eeprom.h"
#include "lib/fmap.h"

int eeprom_mmio_read(struct platform_intf *intf, struct eeprom *eeprom,
                     unsigned int offset, unsigned int len, void *data)
{
	if (offset + len > eeprom->device->size(intf, eeprom))
		return -1;

	return mmio_read(intf, eeprom->addr.mmio + offset, len, data);
}

struct fmap *eeprom_get_fmap(struct platform_intf *intf, struct eeprom *eeprom)
{
	uint8_t *buf;
	struct fmap *fmap_embedded, *fmap;
	int fmap_offset, fmap_size;
	int len;

	if ((len= eeprom->device->size(intf, eeprom)) < 0)
		return NULL;

	buf = mosys_malloc(len);

	if (eeprom->device->read(intf, eeprom, 0, (unsigned int)len, buf) < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read device\n", __func__);
		return NULL;
	}

	if ((fmap_offset = fmap_find(buf, len)) < 0) {
		lprintf(LOG_DEBUG, "%s: cannot find fmap\n", __func__);
		return NULL;
	}

	fmap_embedded = (struct fmap *)(buf + fmap_offset);
	fmap_size = sizeof(*fmap) + (fmap_embedded->nareas *
	                             sizeof(fmap_embedded->areas[0]));
	fmap = malloc(fmap_size);
	memcpy(fmap, fmap_embedded, fmap_size);

	free(buf);
	return fmap;
}
