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

#include <inttypes.h>

#include "mosys/callbacks.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/ite/it8772.h"

#include "lib/acpi.h"
#include "lib/math.h"
#include "lib/sensors.h"

static uint16_t sio_port;

int kiev_superio_setup(struct platform_intf *intf)
{
	if (it8772_get_sioport(intf, &sio_port) <= 0) {
		lprintf(LOG_ERR, "%s: Cannot find Super IO port\n", __func__);
		return -1;
	}
	return 0;
}

void kiev_superio_destroy(struct platform_intf *intf)
{
	it8772_exit(intf, &sio_port);
}

int kiev_set_fantach(struct platform_intf *intf,
                       const char *fan_name, unsigned int percent)
{
	int rc = 0;

	if (!strcmp(fan_name, "system") || !strcmp(fan_name, "all")) {
		if (percent <= 5 && percent > 0) {
			lprintf(LOG_WARNING, "Warning: fan might stall "
			                     "at %u%%\n", percent);
		}
		rc = it8772_set_pwm(intf, 3, percent);
	} else {
		lprintf(LOG_ERR, "Invalid fan \"%s\"\n", fan_name);
		rc = -1;
	}

	return rc;
}

int kiev_set_fantach_auto(struct platform_intf *intf, const char *fan_name)
{
	int rc = 0;

	if (!strcmp(fan_name, "system") || !strcmp(fan_name, "all")) {
		rc = it8772_set_fan_peci(intf, 3);
		if (!rc)
			rc = it8772_set_fan_control_mode(intf,
			                     3,
			                     IT8772_FAN_CONTROL_PWM_AUTOMATIC);
	} else {
		lprintf(LOG_ERR, "Invalid fan \"%s\"\n", fan_name);
		rc = -1;
	}

	return rc;
}

int kiev_set_fantach_off(struct platform_intf *intf, const char *fan_name)
{
	int rc = 0;

	if (!strcmp(fan_name, "system") || !strcmp(fan_name, "all")) {
		rc = it8772_disable_fan(intf, 3);
	} else {
		lprintf(LOG_ERR, "Invalid fan \"%s\"\n", fan_name);
		rc = -1;
	}

	return rc;
}

struct it8772_priv kiev_system_fan_priv = {
	.fan_poles = 2,
};

static struct sensor kiev_onboard_sensors[] = {
	{
		.name		= "system",
		.type		= SENSOR_TYPE_FANTACH,
		.addr.reg	= IT8772_EC_FANTACH3_READING,
		.read		= it8772_read_fantach,
		.priv		= &kiev_system_fan_priv,
	},
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
#if 0
	{
		.name		= "VCORE_1_1V",
		.type		= SENSOR_TYPE_VOLTAGE,
		.addr.reg	= IT8772_EC_VIN0_READING,
		.read		= it8772_read_voltage,
	},
	{
		.name		= "VDIMM_STR_1_5V",
		.type		= SENSOR_TYPE_VOLTAGE,
		.addr.reg	= IT8772_EC_VIN1_READING,
		.read		= it8772_read_voltage,
	},
	{
		.name		= "12V_SEN",
		.type		= SENSOR_TYPE_VOLTAGE,
		.addr.reg	= IT8772_EC_VIN2_READING,
		.read		= it8772_read_voltage,
	},
	{
		.name		= "5V_SEN",
		.type		= SENSOR_TYPE_VOLTAGE,
		.addr.reg	= IT8772_EC_VIN3_READING,
		.read		= it8772_read_voltage,
	},
	{
		.name		= "VLDT_12",
		.type		= SENSOR_TYPE_VOLTAGE,
		.addr.reg	= IT8772_EC_VIN4_READING,
		.read		= it8772_read_voltage,
	},
	{
		.name		= "VBAT_RTC",
		.type		= SENSOR_TYPE_VOLTAGE,
		.addr.reg	= IT8772_EC_VBAT_READING,
		.read		= it8772_read_voltage,
	},
#endif
	{ NULL },
};

/*
 * kiev_add_sensors - register sensor entries
 *
 * @intf:	platform interface
 * @array:	sensor array
 */
static void kiev_add_sensors(struct platform_intf *intf,
                               struct sensor_array *array)
{
	add_sensors(array, kiev_onboard_sensors,
		    ARRAY_SIZE(kiev_onboard_sensors));
}

struct sensor_cb kiev_sensor_cb = {
	.add_sensors		= kiev_add_sensors,
	.set_fantach		= kiev_set_fantach,
	.set_fantach_auto	= kiev_set_fantach_auto,
	.set_fantach_off	= kiev_set_fantach_off,
};
