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
 */

#ifndef MOSYS_LIB_ACPI_H__
#define MOSYS_LIB_ACPI_H__

/* Chrome OS ACPI stuff */
#define CHROMEOS_ACPI_PATH	"/sys/devices/platform/chromeos_acpi/"
#define CHROMEOS_HWID_MAXLEN	256
#define CHROMEOS_FRID_MAXLEN	256

/* Registers from ACPI 4.0 spec */
#define ACPI_EC_SC	0x66		/* ACPI command/status port */
#define ACPI_EC_DATA	0x62		/* ACPI data port */

/* bits 2 and 7 are ignored in ACPI 4.0 spec */
#define ACPI_EC_OBF	(1 << 0)	/* output buffer full */
#define ACPI_EC_IBF   	(1 << 1)	/* input buffer full */
#define ACPI_EC_CMD	(1 << 3)	/* byte in data reg is command byte */
#define ACPI_EC_BURST	(1 << 4)	/* EC in burst mode */
#define ACPI_EC_SCI_EVT	(1 << 5)	/* SCI event pending */
#define ACPI_EC_SMI_EVT	(1 << 6)	/* SMI event pending */

#define ACPI_RD_EC	0x80		/* read byte from EC address space */
#define ACPI_WR_EC	0x81		/* write a byte to EC address space */
#define ACPI_BE_EC	0x82		/* burst enable */
#define ACPI_BD_EC	0x83		/* burst disable */
#define ACPI_QR_EC	0x84		/* query EC */

/* forward declarations */
struct platform_intf;
struct sensor;
struct sensor_reading;

/*
 * acpi_get_hwid - retrieve hardware ID and store in a newly allocated buffer
 *
 * @buf:	buffer to store hardware ID in
 *
 * returns length of hardware id to indicate success
 * returns <0 to indicate error
 */
extern int acpi_get_hwid(char **buf);

/*
 * acpi_get_frid - retrieve FRID and store in a newly allocated buffer
 *
 * @buf:	buffer to store hardware ID in
 *
 * returns length of hardware id to indicate success
 * returns <0 to indicate error
 */
extern int acpi_get_frid(char **buf);

/*
 * acpi_read_temp - read ACPI thermal_zone temperature
 *
 * @intf:	platform interface
 * @sensor:	sensor struct
 * @reading:	location to store reading
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
extern int acpi_read_temp(struct platform_intf *intf, struct sensor *sensor,
                          struct sensor_reading *reading);

#endif /* MOSYS_LIB_ACPI_H__ */
