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
