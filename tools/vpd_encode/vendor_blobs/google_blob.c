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
 *
 * google_blob_encoder.c: Encoder for a Google-defined binary blobs
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/output.h"

#include "lib/math.h"
#include "lib/string.h"
#include "lib/vpd_binary_blob.h"

#include "symbol.h"	/* FIXME: This is actually in the parent dir. Include
			 * paths for vpd_encode are kind of wonky */

/* create_google_blob_v1_1 - Build binary blob in format specified by vendor
 *
 * @buf:	buffer in which to store the blob
 *
 * returns length of allocated buffer to indicate success
 * returns <0 to indicate failure
 */
static int create_google_blob_v1_1(uint8_t **buf)
{
	struct google_blob_1_1 *blob;
	int len = sizeof(struct google_blob_1_1);
	char *s;

	*buf = mosys_realloc(*buf, len);
	memset(*buf, 0, len);
	blob = (struct google_blob_1_1 *)*buf;

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_PRODUCT_SERIAL_NUMBER"))) {
		memcpy(&blob->product_serial_number[0], s,
		       __min(strlen(s), sizeof(blob->product_serial_number)));
	}

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_PRODUCT_SKU"))) {
		memcpy(&blob->product_sku[0], s,
		       __min(strlen(s), sizeof(blob->product_sku)));
	}

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_UUID"))) {
		uuid_t uu;

		if (uuid_parse(s, uu) < 0) {
			lprintf(LOG_ERR, "%s: Invalid UUID specified\n");
			return -1;
		}

		memcpy(&blob->uuid, &uu, sizeof(uu));
	}


	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_MB_SERIAL_NUMBER"))){
		memcpy(&blob->motherboard_serial_number[0],
		       s, __min(strlen(s),
			        sizeof(blob->motherboard_serial_number)));
	}

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_3G_IMEI"))) {
		memcpy(&blob->imei_3g[0],
		       s, __min(strlen(s), sizeof(blob->imei_3g)));
	}

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_SSD_SERIAL_NUMBER"))) {
		memcpy(&blob->ssd_serial_number[0],
		       s, __min(strlen(s), sizeof(blob->ssd_serial_number)));
	}

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_MEMORY_SERIAL_NUMBER"))) {
		memcpy(&blob->memory_serial_number[0],
		       s, __min(strlen(s),sizeof(blob->memory_serial_number)));
	}

	if ((s = sym2str("CONFIG_GOOGLE_BLOB_V1_1_WLAN_MAC_ADDRESS"))) {
		unsigned char *mac = NULL;
		int len;

		if ((len = nstr2buf(&mac, s, 16, ":-")) < 0)
			return -1;
		memcpy(&blob->wlan_mac_id[0], mac,
		       __min(len, sizeof(blob->wlan_mac_id)));
		free(mac);
	}

	return len;
}

/* build_google_vpd_blob - Build binary blob in format specified by vendor
 *
 * @version:	version of the blob
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int build_google_vpd_blob(double version, char outfile[])
{
	int rc = 0;
	int fd;
	int len = -1;
	uint8_t *buf = NULL;

	fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		lprintf(LOG_ERR, "unable to open file \"%s\": %s\n",
				 outfile, strerror(errno));
		rc = -1;
		goto build_google_blob_exit;
	}

	if (version == 1.1) {
		if ((len = create_google_blob_v1_1(&buf)) < 0) {
			rc = -1;
			goto build_google_blob_exit;
		}
	} else {
		lprintf(LOG_ERR, "%s: google blob version %d "
		                 "unsupported\n", version);
		rc = -ENOSYS;
		goto build_google_blob_exit;
	}

	lprintf(LOG_DEBUG, "%s: writing %lu bytes to %s\n",
	                   __func__, len, outfile);

	if (write(fd, buf, len) != len) {
		lprintf(LOG_ERR,
		        "failed to write %s: %s\n", outfile, strerror(errno));
		rc = -1;
		goto build_google_blob_exit;
	}

build_google_blob_exit:
	if (rc < 0) {
		lprintf(LOG_DEBUG, "failed to create google blob "
		                   "version %d\n", version);
	} else {
		mosys_printf("successfully wrote %s\n", outfile);
	}

	free(buf);
	close(fd);
	return rc;
}
