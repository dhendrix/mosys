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
