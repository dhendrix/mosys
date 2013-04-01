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

#ifndef _U100_H__
#define _U100_H__

#define SANDISK_U100_MODEL_NAME		"SanDisk SSD U100"

/* ATA / SCSI command constants. */
#define U100_SENSE_BUFFER_LENGTH                32
#define U100_PHY_PAGE                           0xa2
#define U100_PROTOCOL_PIO_DATA_OUT              0x05
#define U100_ATA_16                             0x85
#define U100_PROTOCOL_PIO_DATA_IN               0x04
#define U100_CMD_TIMEOUT                        1000

/* PHY log page offsets. */
#define U100_PHY_SPEED_OFFSET                   40
#define U100_PHY_SPEED_MODIFIED_OFFSET          U100_PHY_SPEED_OFFSET + 3

/* SanDisk's internal values, stored in PHY log pages. */
#define U100_SPEED_GEN1                         0x00
#define U100_SPEED_GEN2                         0x01
#define U100_SPEED_GEN3                         0x02
#define U100_PHY_SPEED_MODIFIED_MASK            0x80

extern const char *sandisk_u100_get_fw_version(const char *device);

extern enum storage_phy_speed sandisk_u100_get_phy_speed(const char *device);

extern int sandisk_u100_set_phy_speed(const char *device,
				      enum storage_phy_speed phy_speed);

#endif
