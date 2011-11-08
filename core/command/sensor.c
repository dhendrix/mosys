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

#include <ctype.h>
#include <errno.h>
#include <time.h>

#include <valstr.h>

#include "mosys/globals.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/output.h"
#include "mosys/platform.h"

#include "lib/sensors.h"
#include "lib/string.h"

#define MONITOR_DELAY_DEFAULT	10

static const char *sensor_type_names[] = {
	[SENSOR_TYPE_THERMAL_DEGREES]	= "thermal",
	[SENSOR_TYPE_THERMAL_MARGIN]	= "margin",
	[SENSOR_TYPE_THERMAL_TCONTROL]	= "Tcontrol",
	[SENSOR_TYPE_VOLTAGE]		= "voltage",
	[SENSOR_TYPE_FANTACH]		= "fantach",
	[SENSOR_TYPE_CURRENT]		= "current",
	[SENSOR_TYPE_POWER]		= "power",
};


/*
 * kv_pair_print_sensor  -  print sensor key=value pair
 *
 * @type:	sensor type
 * @name:	sensor name
 * @reading:	sensor reading
 */
static void kv_pair_print_sensor(struct sensor *sensor,
                                 struct sensor_reading *reading)
{
	struct kv_pair *kv;
	const char *mode = NULL;

	if (!sensor || !reading)
		return;

	if (reading->mode)
		mode = val2str(reading->mode, sensor_modes);

	kv = kv_pair_new();

	switch (sensor->type) {
	case SENSOR_TYPE_THERMAL_DEGREES: {
		int int_reading = (int)reading->value;
		kv_pair_add(kv, "type", "thermal");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%d", int_reading);
		kv_pair_add(kv, "units", "degrees C");
		break;
	}
	case SENSOR_TYPE_THERMAL_MARGIN: {
		int int_reading = (int)reading->value;
		kv_pair_add(kv, "type", "thermal");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%d", int_reading);
		kv_pair_add(kv, "units", "margin C");
		break;
	}
	case SENSOR_TYPE_THERMAL_TCONTROL: {
		int int_reading = (int)reading->value;
		kv_pair_add(kv, "type", "thermal");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%d", int_reading);
		kv_pair_add(kv, "units", "Tcontrol");
		break;
	}
	case SENSOR_TYPE_VOLTAGE: {
		kv_pair_add(kv, "type", "voltage");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%.2f", reading->value);
		kv_pair_add(kv, "units", "volts");
		break;
	}
	case SENSOR_TYPE_FANTACH: {
		int int_reading = (int)reading->value;
		kv_pair_add(kv, "type", "fantach");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%d", int_reading);
		kv_pair_add(kv, "units", "RPM");
		if (mode)
			kv_pair_add(kv, "mode", mode);
		break;
	}
	case SENSOR_TYPE_CURRENT: {
		kv_pair_add(kv, "type", "current");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%.2f", reading->value);
		kv_pair_add(kv, "units", "amps");
		break;
	}
	case SENSOR_TYPE_POWER: {
		kv_pair_add(kv, "type", "power");
		kv_pair_add(kv, "name", sensor->name);
		kv_pair_fmt(kv, "reading", "%.2f", reading->value);
		kv_pair_add(kv, "units", "watts");
		break;
	}
	}

	kv_pair_print(kv);
	kv_pair_free(kv);
}


static int sensor_monitor_exec(struct platform_intf *intf,
                               unsigned type_mask, int delay, int iterations,
                               int name_count, char **name_list)
{
	struct sensor *sensor;
	struct sensor_array *sensors;
	time_t start = time(NULL);
	char tm_string[40];
	int do_header = 1;
	int this_delay;
	int found = 0;
	uint32_t i;

	sensors = get_platform_sensors(intf);

	setvbuf(stdout, NULL, _IONBF, 0);

	strftime(tm_string, sizeof(tm_string),
	         "%Y-%m-%d %H:%M:%S", localtime(&start));
	mosys_printf("start: %s\n", tm_string);
	mosys_printf("delay: %d\n", delay);

	/* loop and print the readings */
	for (i = 1; iterations == 0 || i <= iterations; i++) {
		int j = 0;
		for (; (sensor = get_sensor(sensors, j)) != NULL;
		     j++) {
			struct sensor_reading reading;

			memset(&reading, 0, sizeof(reading));

			/* don't do sensors that are not requested */
			if (!(type_mask & sensor->type))
				continue;

			/* make sure we can read from this device */
			if (!sensor->read)
				continue;

			/* make sure we are ok to read this sensor */
			if (sensor->flags & SENSOR_FLAG_VERBOSE_ONLY &&
			    !mosys_get_verbosity())
				continue;

			/* check if this sensor was requested by name */
			if (name_count) {
				int n;
				for (n = 0; n < name_count; n++) {
					if (strncmp(sensor->name, name_list[n],
					            __maxlen(sensor->name,
					            name_list[n])) == 0)
						break;
				}
				if (n >= name_count)
					continue;
			}

			/* get reading */
			if (sensor->read(intf, sensor, &reading) < 0)
				continue;

			if (do_header) {
				found++;
				mosys_printf("%s:%s ",
				            sensor_type_names[sensor->type],
				            sensor->name);
				continue;
			}

			/* print it */
			switch (sensor->type) {
			case SENSOR_TYPE_THERMAL_DEGREES:
			case SENSOR_TYPE_THERMAL_MARGIN:
			case SENSOR_TYPE_THERMAL_TCONTROL:
			case SENSOR_TYPE_FANTACH:
				mosys_printf("%d ", (int)reading.value);
				break;
			case SENSOR_TYPE_VOLTAGE:
			case SENSOR_TYPE_CURRENT:
			case SENSOR_TYPE_POWER:
				mosys_printf("%.2f ", reading.value);
				break;
			default:
				continue;
			}
		}
		mosys_printf("\n");

		/* account for any time spent in the scan */
		this_delay = (int)(start + i * delay - time(NULL));

		if (!found) {
			/* no sensors found during header scan */
			lprintf(LOG_ERR, "No sensors found to monitor\n");
			return -1;
		}
		if (do_header) {
			do_header = 0;
			i--;
			/*
			 * since we have to read a sensor to determine if it
			 * is actually present, wait a second so we don't
			 * re-read too quickly or it may not have valid data
			 */
			sleep(1);
		} else {
			sleep(this_delay);
		}
	}

	return 0;
}

static int sensor_monitor_cmd(struct platform_intf *intf,
                              struct platform_cmd *cmd,
                              int argc, char **argv)
{
	int delay = MONITOR_DELAY_DEFAULT;
	int iterations = 0;
	unsigned type_mask = 0;
	int i;

	if (argc > 0)
		delay = atoi(argv[0]);
	if (argc > 1)
		iterations = atoi(argv[1]);

	if (argc < 3)
		return sensor_monitor_exec(intf, SENSOR_TYPE_ALL,
		                           delay, iterations, 0, NULL);

	if (strncmp(argv[2], "type", 4) == 0) {
		/* arguments are sensor types */
		for (i = 2; i < argc; i++) {
			if (strncmp(argv[i], "thermal", 7) == 0)
				type_mask |= SENSOR_TYPE_THERMAL;
			else if (strncmp(argv[i], "voltage", 7) == 0)
				type_mask |= SENSOR_TYPE_VOLTAGE;
			else if (strncmp(argv[i], "fantach", 7) == 0)
				type_mask |= SENSOR_TYPE_FANTACH;
			else if (strncmp(argv[i], "current", 7) == 0)
				type_mask |= SENSOR_TYPE_CURRENT;
			else if (strncmp(argv[i], "power", 5) == 0)
				type_mask |= SENSOR_TYPE_POWER;
			else if (strncmp(argv[i], "all", 3) == 0)
				type_mask |= SENSOR_TYPE_ALL;
		}
		return sensor_monitor_exec(intf, type_mask, delay,
		                           iterations, 0, NULL);
	} else if (strncmp(argv[2], "name", 4) == 0) {
		/* arguments are sensor names */
		return sensor_monitor_exec(intf, SENSOR_TYPE_ALL, delay,
		                           iterations, argc-3, &(argv[3]));
	}

	platform_cmd_usage(cmd);
	return -1;
}

static int sensor_print_exec(struct platform_intf *intf,
                             unsigned int type_mask)
{
	struct sensor_array *sensors;
	struct sensor *sensor;
	int i, count = 0;

	sensors = get_platform_sensors(intf);

	for (i = 0; (sensor = get_sensor(sensors, i)) != NULL; i++) {
		struct sensor_reading reading;

		/* don't do sensors that are not requested */
		if (!(type_mask & sensor->type))
			continue;

		/* make sure we can read from this device */
		if (!sensor->read)
			continue;

		/* make sure we are ok to read this sensor */
		if (sensor->flags & SENSOR_FLAG_VERBOSE_ONLY &&
		    !mosys_get_verbosity())
			continue;

		/* get reading */
		if (sensor->read(intf, sensor, &reading) < 0)
			continue;

		/* print it */
		kv_pair_print_sensor(sensor, &reading);

		count++;
	}

	if (!count)
		return -ENOSYS;
	return 0;
}

static int sensor_print_fantach_cmd(struct platform_intf *intf,
                                    struct platform_cmd *cmd,
                                    int argc, char **argv)
{
	return sensor_print_exec(intf, SENSOR_TYPE_FANTACH);
}

static int sensor_print_thermal_cmd(struct platform_intf *intf,
                                    struct platform_cmd *cmd,
                                    int argc, char **argv)
{
	return sensor_print_exec(intf, SENSOR_TYPE_THERMAL);
}

static int sensor_print_voltage_cmd(struct platform_intf *intf,
                                    struct platform_cmd *cmd,
                                    int argc, char **argv)
{
	return sensor_print_exec(intf, SENSOR_TYPE_VOLTAGE);
}

static int sensor_print_all_cmd(struct platform_intf *intf,
                                struct platform_cmd *cmd,
                                int argc, char **argv)
{
	return sensor_print_exec(intf, SENSOR_TYPE_ALL);
}

/*
 * Syntax: mosys sensor set fan <name> <spec>
 * If "all" is used for name, apply to all sensor devices
 * If "spec" is set to auto, use automatic (hardware-controlled) setting
 */
static int sensor_set_fantach_cmd(struct platform_intf *intf,
                              struct platform_cmd *cmd,
                              int argc, char **argv)
{
	int ret = 0;
	const char *name = NULL; 	/* Name of fan */
	const char *spec = NULL; 	/* User-specified value */
	unsigned int percent;

	if (argc != 2) {
		platform_cmd_usage(cmd);
		return -1;
	}

	name = argv[0];
	spec = argv[1];

	if (!strncmp(spec, "auto", 4)) {
		if (!intf->cb->sensor->set_fantach_auto)
			return -ENOSYS;
		lprintf(LOG_DEBUG, "Setting fan %s to auto mode\n", name);
		ret = intf->cb->sensor->set_fantach_auto(intf, name);
	} else if (!strncmp(spec, "off", 3)) {
		if (!intf->cb->sensor->set_fantach_off)
			return -ENOSYS;
		lprintf(LOG_DEBUG, "Disabling fan %s\n", name);
		ret = intf->cb->sensor->set_fantach_off(intf, name);
	} else {
		if (!intf->cb->sensor->set_fantach)
			return -ENOSYS;

		percent = strtoul(spec, NULL, 0);

		if ((percent > 100) || (percent < 0)) {
			lprintf(LOG_ERR, "Invalid percentage %u%%\n",
			        percent);
			return -1;
		}

		lprintf(LOG_DEBUG, "Setting fan tach to %u\%\n", percent);
		ret = intf->cb->sensor->set_fantach(intf, name, percent);
	}

	return ret;
}

struct platform_cmd sensor_print_cmds[] = {
	{
		.name	= "fantach",
		.desc	= "Print Fan Information",
		.usage	= "<name>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = sensor_print_fantach_cmd }
	},
	{
		.name	= "thermal",
		.desc	= "Print Thermal Information",
		.usage	= "<name>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = sensor_print_thermal_cmd }
	},
	{
		.name	= "voltage",
		.desc	= "Print Voltage Information",
		.usage	= "<name>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = sensor_print_voltage_cmd }
	},
	{
		.name	= "all",
		.desc	= "Print All Sensors",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = sensor_print_all_cmd }
	},
	{ NULL }
};

struct platform_cmd sensor_set_cmds[] = {
	{
		.name	= "fantach",
		.desc	= "Set Fan Speed",
		.usage	= "<fan_name> <percent>",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = sensor_set_fantach_cmd }
	},
#if 0
	{
		.name	= "thermal",
		.desc	= "Modify thermal parameters",
		.usage	= "<name> <parameter> <value>",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = sensor_set_thermal_cmd }
	},
#endif
#if 0
	{
		.name	= "voltage",
		.desc	= "Modify voltage parameters",
		.usage	= "<name> <parameter> <value>",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = sensor_set_voltage_cmd }
	},
#endif
	{ NULL }
};

struct platform_cmd sensor_cmds[] = {
	{
		.name	= "print",
		.desc	= "Print Sensor Info",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = sensor_print_cmds }
	},
	{
		.name	= "set",
		.desc	= "Set Sensor Parameters",
		.type	= ARG_TYPE_SUB,
		.arg	= { .sub = sensor_set_cmds }
	},
	{
		.name	= "monitor",
		.desc	= "Monitor Sensor Readings",
		.usage	= "[delay] [count] [type|name] [sensor types or names...]",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = sensor_monitor_cmd }
	},
	{ NULL }
};

struct platform_cmd cmd_sensor = {
	.name	= "sensor",
	.desc	= "Sensor Information",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = sensor_cmds }
};
