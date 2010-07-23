/* Copyright 2010, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *    * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>

#include "mosys/kv_pair.h"
#include "mosys/string.h"

#include "lib/crypto.h"
#include "lib/fmap.h"

static const struct valstr flag_lut[] = {
	{ 1 << 0, "static" },
	{ 1 << 1, "compressed" },
};

off_t fmap_find(const uint8_t *image, size_t image_len)
{
	off_t offset;
	uint64_t sig;

	memcpy(&sig, FMAP_SIGNATURE, strlen(FMAP_SIGNATURE));

	/* Find 4-byte aligned FMAP signature within image */
	for (offset = 0; offset < image_len; offset += 4) {
		if (!memcmp(&image[offset], &sig, sizeof(sig))) {
			break;
		}
	}

	if (offset < image_len)
		return offset;
	else
		return -1;
}

int fmap_print(const struct fmap *fmap)
{
	int i;
	struct kv_pair *kv = NULL;

        kv = kv_pair_new();
	if (!kv)
		return -1;

	kv_pair_fmt(kv, "fmap_signature", "0x%016llx",
	            (unsigned long long)fmap->signature);
	kv_pair_fmt(kv, "fmap_ver_major", "%d", fmap->ver_major);
	kv_pair_fmt(kv, "fmap_ver_minor","%d", fmap->ver_minor);
	kv_pair_fmt(kv, "fmap_base", "0x%016llx",
	            (unsigned long long)fmap->base);
	kv_pair_fmt(kv, "fmap_size", "0x%04x", fmap->size);
	kv_pair_fmt(kv, "fmap_name", "%s", fmap->name);
	kv_pair_fmt(kv, "fmap_nareas", "%d", fmap->nareas);
	kv_pair_print(kv);
	kv_pair_free(kv);

	for (i = 0; i < fmap->nareas; i++) {
		struct kv_pair *kv;
		uint16_t flags;
		char str[256] = { '\0' };
		int j;

		kv = kv_pair_new();
		if (!kv)
			return -1;

		kv_pair_fmt(kv, "area_offset", "0x%08x",
				fmap->areas[i].offset);
		kv_pair_fmt(kv, "area_size", "0x%08x",
				fmap->areas[i].size);
		kv_pair_fmt(kv, "area_name", "%s",
				fmap->areas[i].name);
		kv_pair_fmt(kv, "area_flags_raw", "0x%02x",
				fmap->areas[i].flags);

		/* Print descriptive strings for flags rather than the field */
		flags = fmap->areas[i].flags;
		for (j = 0; j < 16; j++) {
			if (!flags)
				break;

			if (flags & (1 << j)) {
				const char *tmp = val2str(1 << j, flag_lut);

				strncat(str, tmp, strlen(tmp));
				flags &= ~(1 << j);
				if (flags)
					strncat(str, ",", 1);
			}
		}
		kv_pair_fmt(kv, "area_flags", "%s", str );

		kv_pair_print(kv);
		kv_pair_free(kv);
	}

	return 0;
}

int fmap_get_csum(const uint8_t *image, size_t image_len,
                  uint8_t **digest, struct crypto_algo *crypto)
{
	int i;
	struct fmap *fmap;
	int fmap_offset;

	if ((image == NULL))
		return -1;

	if ((fmap_offset = fmap_find(image, image_len)) < 0)
		return -1;
	fmap = image + fmap_offset;

	crypto->init(crypto->ctx);

	/* Iterate through flash map and calculate the checksum piece-wise. */
	for (i = 0; i < fmap->nareas; i++) {
		/* skip non-static areas */
		if (!(fmap->areas[i].flags & FMAP_AREA_STATIC))
			continue;

		/* sanity check the offset */
		if (fmap->areas[i].size + fmap->areas[i].offset > image_len) {
			fprintf(stderr,
			        "(%s) invalid parameter detected in area %d\n",
			        __func__, i);
			return -1;
		}

		crypto->update(crypto->ctx,
		               image + fmap->areas[i].offset,
		               fmap->areas[i].size);
	}

	crypto->final(crypto->ctx);
	*digest = crypto->get_digest(crypto);
	return 0;
}
