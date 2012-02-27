/* Copyright 2012, Google Inc.
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

#include <fmap.h>

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/mmio.h"

#include "lib/eeprom.h"

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
