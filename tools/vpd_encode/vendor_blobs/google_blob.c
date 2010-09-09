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

#include "mosys/log.h"
#include "mosys/output.h"

#include "lib/math.h"
#include "lib/string.h"
#include "lib/vpd_binary_blob.h"

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
	unsigned char *tmpstr;
	int tmplen;

	/* FIXME: Add sanity checking */
	*buf = realloc(*buf, len);
	memset(*buf, 0, len);
	blob = (struct google_blob_1_1 *)*buf;

#ifdef CONFIG_GOOGLE_BLOB_V1_1_PRODUCT_SERIAL_NUMBER
	tmpstr = format_string(CONFIG_GOOGLE_BLOB_V1_1_PRODUCT_SERIAL_NUMBER);
	memcpy(&blob->product_serial_number[0], tmpstr,
	       __min(strlen(tmpstr), sizeof(blob->product_serial_number)));
	free(tmpstr);
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_PRODUCT_SKU
	tmpstr = format_string(CONFIG_GOOGLE_BLOB_V1_1_PRODUCT_SKU);
	memcpy(&blob->product_sku[0], tmpstr,
	       __min(strlen(tmpstr), sizeof(blob->product_sku)));
	free(tmpstr);
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_UUID
	{
		uuid_t uu;

		if (uuid_parse(CONFIG_GOOGLE_BLOB_V1_1_UUID, uu) < 0) {
			lprintf(LOG_ERR, "%s: Invalid UUID specified\n");
			return -1;
		}

		memcpy(&blob->uuid, &uu, sizeof(uu));
	}
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_MB_SERIAL_NUMBER
	tmpstr = format_string(CONFIG_GOOGLE_BLOB_V1_1_MB_SERIAL_NUMBER);
	memcpy(&blob->motherboard_serial_number[0],
	       tmpstr, __min(tmplen, sizeof(blob->motherboard_serial_number)));
	free(tmpstr);
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_3G_IMEI
	if ((tmplen = nstr2buf(&tmpstr,
	                 CONFIG_GOOGLE_BLOB_V1_1_3G_IMEI, 16, "-")) < 0)
		return -1;
	memcpy(&blob->imei_3g[0], tmpstr,
	       __min(tmplen, sizeof(blob->imei_3g)));
	free(tmpstr);
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_SSD_SERIAL_NUMBER
	tmpstr = format_string(CONFIG_GOOGLE_BLOB_V1_1_SSD_SERIAL_NUMBER);
	memcpy(&blob->ssd_serial_number[0],
	       tmpstr, __min(strlen(tmpstr), sizeof(blob->ssd_serial_number)));
	free(tmpstr);
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_MEMORY_SERIAL_NUMBER
	tmpstr = format_string(CONFIG_GOOGLE_BLOB_V1_1_MEMORY_SERIAL_NUMBER);
	memcpy(&blob->memory_serial_number[0],
	       tmpstr, __min(strlen(tmpstr),
	       sizeof(blob->memory_serial_number)));
	free(tmpstr);
#endif
#ifdef CONFIG_GOOGLE_BLOB_V1_1_WLAN_MAC_ADDRESS
	if ((tmplen = nstr2buf(&tmpstr,
	                CONFIG_GOOGLE_BLOB_V1_1_WLAN_MAC_ADDRESS, 16, ":")) < 0)
		return -1;
	memcpy(&blob->wlan_mac_id[0],
	       tmpstr, __min(tmplen, sizeof(blob->wlan_mac_id)));
	free(tmpstr);
#endif

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
