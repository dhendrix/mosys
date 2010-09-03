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
 * binary_blob.c: helper functions for handling binary blobs
 */

#include <stdlib.h>
#include <uuid/uuid.h>

#include "mosys/log.h"

#include "lib/string.h"
#include "lib/vpd_tables.h"
#include "lib/vpd_binary_blob.h"

int print_agz_blob_v3(uint8_t *data, uint32_t size, struct kv_pair *kv)
{
	struct agz_blob_0_3 *agz_blob = data;
	char s[37];
	char *s2;

	if (size != sizeof(*agz_blob)) {
		lprintf(LOG_DEBUG, "AGZ binary blob expected size: %lu, "
		                   "got: %lu\n", sizeof(*agz_blob), size);
		return -1;
	}

	kv_pair_add(kv, "blob_type", "agz_blob");

	snprintf(s, sizeof(agz_blob->product_name) + 1, "%s", agz_blob->product_name);
	kv_pair_add(kv, "product_name", s);

	snprintf(s, sizeof(agz_blob->product_manufacturer) + 1,
	         "%s", agz_blob->product_manufacturer);
	kv_pair_add(kv, "product_manufacturer", s);

	uuid_unparse(agz_blob->uuid, s);
	kv_pair_add(kv, "product_uuid", s);

	s2 = buf2str(agz_blob->motherboard_serial_number,
	                 sizeof(agz_blob->motherboard_serial_number));
	kv_pair_add(kv, "motherboard_serial_number", s2);
	free(s2);

	s2 = buf2nicid(agz_blob->esn_3g, NIC_ID_IMEI);
	kv_pair_add(kv, "3g_esn", s2);
	free(s2);

	s2 = buf2nicid(agz_blob->wlan_mac_id, NIC_ID_IEEE802);
	kv_pair_add(kv, "wlan_macid", s2);
	free(s2);

	snprintf(s, sizeof(agz_blob->country_code) + 1,
	         "%s", agz_blob->country_code);
	kv_pair_add(kv, "country_code", s);

	snprintf(s, sizeof( agz_blob->product_serial_number) + 1,
	         "%s", agz_blob->product_serial_number);
	kv_pair_fmt(kv, "product_serial_number", s);

	return 0;
}

int print_agz_blob_v5(uint8_t *data, uint32_t size, struct kv_pair *kv)
{
	struct agz_blob_0_5 *agz_blob = data;
	char s[37];
	char *s2;

	if (size != sizeof(*agz_blob)) {
		lprintf(LOG_DEBUG, "AGZ binary blob expected size: %lu, "
		                   "got: %lu\n", sizeof(*agz_blob), size);
		return -1;
	}

	kv_pair_add(kv, "blob_type", "agz_blob");

	snprintf(s, sizeof(agz_blob->product_name) + 1, "%s", agz_blob->product_name);
	kv_pair_add(kv, "product_name", s);

	snprintf(s, sizeof(agz_blob->product_manufacturer) + 1,
	         "%s", agz_blob->product_manufacturer);
	kv_pair_add(kv, "product_manufacturer", s);

	uuid_unparse(agz_blob->uuid, s);
	kv_pair_add(kv, "product_uuid", s);

	snprintf(s, sizeof(agz_blob->motherboard_serial_number) + 1,
	         "%s", agz_blob->motherboard_serial_number);
	kv_pair_add(kv, "motherboard_serial_number", s);

	snprintf(s, sizeof(agz_blob->esn_3g) + 1, "%s", agz_blob->esn_3g);
	kv_pair_add(kv, "3g_esn", s);

	s2 = buf2nicid(agz_blob->wlan_mac_id, NIC_ID_IEEE802);
	kv_pair_add(kv, "wlan_macid", s2);
	free(s2);

	snprintf(s, sizeof(agz_blob->country_code) + 1,
	         "%s", agz_blob->country_code);
	kv_pair_add(kv, "country_code", s);

	snprintf(s, sizeof( agz_blob->product_serial_number) + 1,
	         "%s", agz_blob->product_serial_number);
	kv_pair_fmt(kv, "product_serial_number", s);

	return 0;
}
