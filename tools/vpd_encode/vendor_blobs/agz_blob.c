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
 * agz_vendor_blob_encoder.c: Encoder for a vendor-defined AGZ binary blob
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

#include "vendor_blobs.h"

#ifndef CONFIG_AGZ_BLOB_FILENAME
#define CONFIG_AGZ_BLOB_FILENAME	"agz_blob.bin"
#endif

/* build_agz_vendor_blob - Build binary blob in format specified by vendor
 *
 * @buf:	buffer in which to store the blob
 *
 * returns length of allocated buffer to indicate success
 * returns <0 to indicate failure
 */
static int create_agz_blob_v3(uint8_t **buf)
{
	struct agz_blob_0_3 *blob;
	int len = sizeof(struct agz_blob_0_3);
	unsigned char *tmpstr;
	int i, tmplen;

	/* FIXME: Add sanity checking */
	*buf = realloc(*buf, len);
	memset(*buf, 0, len);
	blob = (struct agz_blob_0_3 *)*buf;

#ifdef CONFIG_AGZ_BLOB_PRODUCT_NAME
	tmpstr = format_string(CONFIG_AGZ_BLOB_PRODUCT_NAME);
	memcpy(&blob->product_name[0], tmpstr,
	       __min(strlen(tmpstr), sizeof(blob->product_name)));
	free(tmpstr);
#endif
#ifdef CONFIG_AGZ_BLOB_PRODUCT_MFG
	tmpstr = format_string(CONFIG_AGZ_BLOB_PRODUCT_MFG);
	memcpy(&blob->product_manufacturer[0], tmpstr,
	       __min(strlen(tmpstr), sizeof(blob->product_manufacturer)));
	free(tmpstr);
#endif
#ifdef CONFIG_AGZ_BLOB_UUID
	{
		uuid_t uu;

		if (uuid_parse(CONFIG_AGZ_BLOB_UUID, uu) < 0) {
			lprintf(LOG_ERR, "%s: Invalid UUID specified\n");
			return -1;
		}

		memcpy(&blob->uuid, &uu, sizeof(uu));
	}
#endif
#ifdef CONFIG_AGZ_BLOB_MB_SERIAL_NUMBER
	if ((tmplen = nstr2buf(&tmpstr, 
	                       CONFIG_AGZ_BLOB_MB_SERIAL_NUMBER, 16, "")) < 0)
		return -1;
	memcpy(&blob->motherboard_serial_number[0],
	       tmpstr, __min(tmplen, sizeof(blob->motherboard_serial_number)));
	free(tmpstr);
#endif

#ifdef CONFIG_AGZ_BLOB_3G_ESN
	if ((tmplen = nstr2buf(&tmpstr, CONFIG_AGZ_BLOB_3G_ESN, 16, "-")) < 0)
		return -1;
	memcpy(&blob->esn_3g[0], tmpstr,
	       __min(tmplen, sizeof(blob->esn_3g)));
	free(tmpstr);
#endif
#ifdef CONFIG_AGZ_BLOB_LOCAL_COUNTRY_CODE
	tmpstr = format_string(CONFIG_AGZ_BLOB_LOCAL_COUNTRY_CODE);
	memcpy(&blob->country_code[0], tmpstr,
	       __min(strlen(tmpstr), sizeof(blob->country_code)));
	free(tmpstr);
#endif
#ifdef CONFIG_AGZ_BLOB_WLAN_MAC_ADDRESS
	if ((tmplen = nstr2buf(&tmpstr,
	                       CONFIG_AGZ_BLOB_WLAN_MAC_ADDRESS, 16, ":")) < 0)
		return -1;
	memcpy(&blob->wlan_mac_id[0],
	       tmpstr, __min(tmplen, sizeof(blob->wlan_mac_id)));
	free(tmpstr);
#endif
#ifdef CONFIG_AGZ_BLOB_PRODUCT_SERIAL_NUMBER
	tmpstr = format_string(CONFIG_AGZ_BLOB_PRODUCT_SERIAL_NUMBER);
	memcpy(&blob->product_serial_number[0],
	       tmpstr, __min(strlen(tmpstr), sizeof(blob->product_serial_number)));
	free(tmpstr);
#endif

	return len;
}

/* build_agz_vendor_blob - Build binary blob in format specified by vendor
 *
 * @version:	version of the blob
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int build_agz_vendor_blob(int version)
{
	char outfile[] = CONFIG_AGZ_BLOB_FILENAME;
	int rc = 0;
	int fd;
	int len = -1;
	int i;
	uint8_t *buf = NULL;

	fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		lprintf(LOG_ERR, "unable to open file \"%s\": %s\n",
				 outfile, strerror(errno));
		rc = -1;
		goto build_agz_blob_exit;
	}

	switch(version) {
	case 3:
		if ((len = create_agz_blob_v3(&buf)) < 0) {
			rc = -1;
			goto build_agz_blob_exit;
		}
		break;
	default:
		lprintf(LOG_ERR, "%s: agz blob version %d "
		                 "unsupported\n", version);
		rc = -ENOSYS;
		goto build_agz_blob_exit;
	}

	lprintf(LOG_DEBUG, "%s: writing %lu bytes to %s\n",
	                   __func__, len, outfile);

	if (write(fd, buf, len) != len) {
		lprintf(LOG_ERR,
		        "failed to write %s: %s\n", outfile, strerror(errno));
		rc = -1;
		goto build_agz_blob_exit;
	}

build_agz_blob_exit:
	if (rc < 0) {
		lprintf(LOG_DEBUG, "failed to create agz blob "
		                   "version %d\n", version);
	} else {
		mosys_printf("successfully wrote %s\n", outfile);
	}

	free(buf);
	close(fd);
	return rc;
}
