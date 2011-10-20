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

#include <inttypes.h>

#include "mosys/callbacks.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/ite/it8772.h"

#include "lib/math.h"
#include "lib/sensors.h"

static uint16_t sio_port;

int stumpy_superio_setup(struct platform_intf *intf)
{
	if (it8772_get_sioport(intf, &sio_port) <= 0) {
		lprintf(LOG_ERR, "%s: Cannot find Super IO port\n", __func__);
		return -1;
	}
	return 0;
}

void stumpy_superio_destroy(struct platform_intf *intf)
{
	it8772_exit(intf, &sio_port);
}

int stumpy_set_fantach(struct platform_intf *intf,
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

int stumpy_set_fantach_auto(struct platform_intf *intf, const char *fan_name)
{
	int rc = 0;

	if (!strcmp(fan_name, "system") || !strcmp(fan_name, "all")) {
		rc = it8772_set_fan_peci(intf, 3);
	} else {
		lprintf(LOG_ERR, "Invalid fan \"%s\"\n", fan_name);
		rc = -1;
	}

	return rc;
}

int stumpy_set_fantach_off(struct platform_intf *intf, const char *fan_name)
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

struct it8772_priv system_fan_priv = {
	.fan_poles = 2,
};

static struct sensor stumpy_onboard_sensors[] = {
	{
		.name		= "system",
		.type		= SENSOR_TYPE_FANTACH,
		.addr.reg	= IT8772_EC_FANTACH3_READING,
		.read		= it8772_read_fantach,
		.priv		= &system_fan_priv,
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
 * stumpy_add_sensors - register sensor entries
 *
 * @intf:	platform interface
 * @array:	sensor array
 */
static void stumpy_add_sensors(struct platform_intf *intf,
                               struct sensor_array *array)
{
	add_sensors(array, stumpy_onboard_sensors,
		    ARRAY_SIZE(stumpy_onboard_sensors));
}

struct sensor_cb stumpy_sensor_cb = {
	.add_sensors		= stumpy_add_sensors,
	.set_fantach		= stumpy_set_fantach,
	.set_fantach_auto	= stumpy_set_fantach_auto,
	.set_fantach_off	= stumpy_set_fantach_off,
};
