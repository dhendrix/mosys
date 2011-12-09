/*
 * Copyright (C) 2011 Google Inc.
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

#include "mosys/platform.h"

#include "lib/acpi.h"
#include "lib/math.h"
#include "lib/sensors.h"

static struct sensor lumpy_onboard_sensors[] = {
	{
		.name		= "temp0",
		.type		= SENSOR_TYPE_THERMAL_DEGREES,
		.addr.sysfs_num	= 0,
		.read		= acpi_read_temp,
	},

	/* Digital Thermal Sensor readings from CPU package 0 */
	{
		.name		= "package0",
		.type		= SENSOR_TYPE_THERMAL_DEGREES,
		.addr.dts	= { 0, 1 },
		.read		= dts_read,
	},
	{
		.name		= "core0",
		.type		= SENSOR_TYPE_THERMAL_DEGREES,
		.addr.dts	= { 0, 2 },
		.read		= dts_read,
	},
	{
		.name		= "core1",
		.type		= SENSOR_TYPE_THERMAL_DEGREES,
		.addr.dts	= { 0, 3 },
		.read		= dts_read,
	},
	{ NULL },
};

/*
 * lumpy_add_sensors - register sensor entries
 *
 * @intf:	platform interface
 * @array:	sensor array
 */
static void lumpy_add_sensors(struct platform_intf *intf,
                               struct sensor_array *array)
{
	add_sensors(array, lumpy_onboard_sensors,
		    ARRAY_SIZE(lumpy_onboard_sensors));
}

struct sensor_cb lumpy_sensor_cb = {
	.add_sensors		= lumpy_add_sensors,
};
