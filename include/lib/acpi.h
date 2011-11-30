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
