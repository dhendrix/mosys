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
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "lib/ata.h"
#include "drivers/sandisk/u100.h"

/*
 * sandisk_u100_read_write_log_page - Reads or writes a log page on SSD.
 *
 * @device:	Path of device, ex. "/dev/sda".
 * @page_num:	Page number to read / write.
 * @page_buf:	Buffer to read to or write from.
 * @write:	False to read from page, true to write to page.
 *
 * Returns 0 in case of success or < 0 for failure..
 */
static int sandisk_u100_read_write_log_page(const char *device,
					    unsigned char page_num,
					    unsigned char *page_buf,
					    bool write)
{
	unsigned char cdb[ATAPI_CDB_LEN] = {0};
	unsigned char sense_buffer[U100_SENSE_BUFFER_LENGTH] = {0};
	sg_io_hdr_t io_hdr = {0};

	int rc;
	int fd;
	unsigned char cmd, protocol;
	int direction;

	lprintf(LOG_DEBUG, "%s: device / rw: %s %d\n", __func__, device, write);
	fd = open(device, O_RDONLY);
	if (fd == -1) {
		lprintf(LOG_ERR, "%s: failed to open device: %s\n", __func__,
								    device);
		return -1;
	}

	if (write) {
		cmd = ATA_CMD_WRITE_LOG_EXT;
		direction = SG_DXFER_TO_DEV;
		protocol = U100_PROTOCOL_PIO_DATA_OUT;
	} else {
		cmd = ATA_CMD_READ_LOG_EXT;
		direction = SG_DXFER_FROM_DEV;
		protocol = U100_PROTOCOL_PIO_DATA_IN;
	}

	/* cdb[0]: ATA PASS THROUGH (16) SCSI command opcode byte (0x85)
	   cdb[1]: multiple_count, protocol + extend
	   cdb[2]: offline, ck_cond, t_dir, byte_block + t_length
	   cdb[3]: features (15:8)
	   cdb[4]: features (7:0)
	   cdb[5]: sector_count (15:8)
	   cdb[6]: sector_count (7:0)
	   cdb[7]: lba_low (15:8)
	   cdb[8]: lba_low (7:0)
	   cdb[9]: lba_mid (15:8)
	   cdb[10]: lba_mid (7:0)
	   cdb[11]: lba_high (15:8)
	   cdb[12]: lba_high (7:0)
	   cdb[13]: device
	   cdb[14]: (ata) command
	   cdb[15]: control (SCSI, leave as zero) */
	cdb[0] = U100_ATA_16;
	cdb[1] = (protocol << 1) | 1;
	cdb[2] = (!write) << 3 | 1 << 2 | 2;
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0;
	cdb[6] = 1;
	cdb[7] = 0;
	cdb[8] = page_num;
	cdb[9] = 0;
	cdb[10] = 0;
	cdb[11] = 0;
	cdb[12] = 0;
	cdb[13] = 0;
	cdb[14] = cmd;

	io_hdr.interface_id = 'S';
	io_hdr.mx_sb_len = sizeof(sense_buffer);
	io_hdr.sbp = sense_buffer;
	io_hdr.timeout = U100_CMD_TIMEOUT;
	io_hdr.cmdp = cdb;
	io_hdr.cmd_len = sizeof(cdb);
	io_hdr.dxferp = (void *)page_buf;
	io_hdr.dxfer_len = ATA_SECT_SIZE;
	io_hdr.dxfer_direction = direction;

	rc = ioctl(fd, SG_IO, &io_hdr);
	close(fd);

	return rc;
}

/*
 * sandisk_u100_get_phy_speed - Gets current SSD PHY speed setting.
 *
 * @device:	Path of device, ex. "/dev/sda".
 *
 * Returns storage_phy_speed value corresponding to current speed setting.
 */
enum storage_phy_speed sandisk_u100_get_phy_speed(const char *device)
{
	unsigned char data_buf[ATA_SECT_SIZE];
	enum storage_phy_speed phy_speed = PHY_SPEED_UNKNOWN;
	int res;
	char path[PATH_MAX];

	sprintf(path, "%s/%s", mosys_get_root_prefix(), device);
	lprintf(LOG_DEBUG, "%s: phy_speed: %s\n", __func__, path);

	/* Read page containg PHY params. */
	res = sandisk_u100_read_write_log_page(path, U100_PHY_PAGE,
					       data_buf, false);

	/* Translate to return value. */
	if (res == 0) {
		switch(data_buf[U100_PHY_SPEED_OFFSET]) {
			case U100_SPEED_GEN1:
				phy_speed = PHY_SPEED_SATA1;
				break;
			case U100_SPEED_GEN2:
				phy_speed = PHY_SPEED_SATA2;
				break;
			case U100_SPEED_GEN3:
				phy_speed = PHY_SPEED_SATA3;
				break;
			default:
				phy_speed = PHY_SPEED_UNKNOWN;
				break;
		}
	}

	lprintf(LOG_DEBUG, "%s: return phy_speed: %d\n", __func__, phy_speed);
	return phy_speed;
}

/*
 * sandisk_u100_set_phy_speed - Sets SSD PHY speed setting.
 *
 * @device:	Path of device, ex. "/dev/sda".
 * @phy_speed:	PHY speed to set on device.
 *
 * Returns storage_phy_speed value corresponding to current speed setting.
 * Note: SSD requires a power-cycle to reflect new PHY speed setting after
 * write.
 */
int sandisk_u100_set_phy_speed(const char *device,
			       enum storage_phy_speed phy_speed)
{
	unsigned char data_buf[ATA_SECT_SIZE];
	unsigned char write_speed;
	int res = 0;
	char path[PATH_MAX];

	sprintf(path, "%s/%s", mosys_get_root_prefix(), device);
	lprintf(LOG_DEBUG, "%s: phy_speed: %s %d\n", __func__, path,
						       phy_speed);

	/* Read page containing PHY params. */
	res = sandisk_u100_read_write_log_page(path, U100_PHY_PAGE,
					       data_buf, false);

	switch(phy_speed) {
		case PHY_SPEED_SATA1:
			write_speed = U100_SPEED_GEN1;
			break;
		case PHY_SPEED_SATA2:
			write_speed = U100_SPEED_GEN2;
			break;
		case PHY_SPEED_SATA3:
			write_speed = U100_SPEED_GEN3;
			break;
		default:
			res = -1;
			break;
	}

	/* Write back page w/ new PHY speed. */
	if (res == 0) {
		data_buf[U100_PHY_SPEED_OFFSET] = write_speed;
		data_buf[U100_PHY_SPEED_MODIFIED_OFFSET] |=
			U100_PHY_SPEED_MODIFIED_MASK;
		res = sandisk_u100_read_write_log_page(path, U100_PHY_PAGE,
						       data_buf, true);
	}

	lprintf(LOG_DEBUG, "%s: return success: %d\n", __func__, res);
	return res;
}
