/*
 * Copyright 2011, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *    * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/platform.h"

#include "lib/dynamic_array.h"
#include "lib/sensors.h"

static struct sensor_array *platform_sensors;

struct sensor *new_sensor(const char *name, sensor_type type,
                          union sensor_addr addr, sensor_read_function read,
                          sensor_flag_mask flags, void *priv) {
	struct sensor *sensor_data = mosys_malloc(sizeof(struct sensor));
	sensor_data->name = mosys_strdup(name);
	sensor_data->type = type;
	sensor_data->addr = addr;
	sensor_data->read = read;
	sensor_data->flags = flags;
	sensor_data->priv = priv;
	return sensor_data;
}

void free_sensor(struct sensor *sensor) {
	if (!sensor) {
		return;
	}
	free((void*)sensor->name);
	free(sensor);
}

struct sensor_array {
	struct dynamic_array *sensors;
};

/*
 * new_sensor_array - allocate a sensor_array structure with default size
 *
 * returns an allocated sensor_array
 */
struct sensor_array *new_sensor_array(void)
{
	struct sensor_array *array;

	array = mosys_malloc(sizeof(*array));
        array->sensors = new_dynamic_array(sizeof(struct sensor *));
	return array;
}

/*
 * free_sensor_array - free the allocated sensor_array
 *
 * @array:  sensor_array to free
 */
void free_sensor_array(struct sensor_array *array)
{
	if (array == NULL) {
		return;
	}

	free_dynamic_array(array->sensors);
	free(array);
}

/*
 * free_sensor_array_ptr - free the allocated sensor_array pointed to by ptr
 *
 * @ptr:  pointer to sensor_array to free and NULL out
 */
void free_sensor_array_ptr(struct sensor_array **ptr)
{
	if (ptr == NULL) {
		return;
	}

	free_sensor_array(*ptr);
	*ptr = NULL;
}

/*
 * add_sensor - add a sensor to the array
 *
 * @array:  sensor_array to add sensor to
 * @sensor:  sensor to add
 *
 * returns 0 on success, < 0 if failure
 */
int add_sensor(struct sensor_array *array, struct sensor *sensor)
{
	if (!array || !array->sensors || !sensor)
		return -1;

	return dynamic_array_push_back(array->sensors, &sensor);
}

/*
 * add_sensors - add a group of sensors to the array
 *
 * @array:  sensor_array to add sensor to
 * @sensors:  sensors to add
 * @num:  number of sensors to add
 *
 * returns number of sensors added
 * returns <0 to indicate error
 */
size_t add_sensors(struct sensor_array *array,
                   struct sensor *sensors, size_t num)
{
	size_t added;

	if (!array || !array->sensors || !sensors)
		return -1;

	for (added = 0; added < num; added++) {
		if (add_sensor(array, sensors) < 0) {
			break;
		}
		sensors++;
	}

	return added;
}

/*
 * num_sensors - return the number of sensors in given array
 *
 * @array:  sensor_array to obtain number of sensors
 *
 * returns number of sensors in array, -1 if error
 */
size_t num_sensors(struct sensor_array *array)
{
	if (!array || !array->sensors)
		return -1;

	return dynamic_array_size(array->sensors);
}

/*
 * get_sensor - return the requested sensor from the given array
 *
 * @array:  sensor_array to obtain number of sensors
 * @entry:  entry number of the sensor within the array (0 based)
 *
 * returns requested sensor if successful, NULL if error
 */
struct sensor *get_sensor(struct sensor_array *array, size_t entry)
{
	if (!array || !array->sensors)
		return NULL;

	if (entry >= num_sensors(array)) {
		return NULL;
	}

	return dynamic_array_get_entry(array->sensors, entry, struct sensor *);
}

/*
 * get_sensor_by_name_and_type - return the requested sensor
 *         from the given array, if name and type matched
 *
 * @array:  sensor_array to obtain number of sensors
 * @name:   sensor's name
 * @type:   enum sensor_type
 *
 * returns requested sensor if successful, NULL if error
 */
struct sensor *get_sensor_by_name_and_type(struct sensor_array *array,
                                           const char *name,
                                           sensor_type type)
{
	struct sensor *ret = NULL;

	if (array && array->sensors && name) {
		int num = num_sensors(array);
		int i;

		for(i = 0; i < num; i++) {
			struct sensor *sr = get_sensor(array, i);
			if (sr && sr->type == type &&
			    sr->name && !strcmp(sr->name, name)) {
				ret = sr;
				break;
			}
		}
	}

	return ret;
}

struct sensor_array *get_platform_sensors(struct platform_intf *intf)
{
	if (platform_sensors) {
		return platform_sensors;
	}

	/* Allocate and fill in sensor array. Also, ensure sensor_array is
	 * free'd when mosys is torn down. */
	platform_sensors = new_sensor_array();
	add_destroy_callback((destroy_callback)free_sensor_array_ptr,
	                     &platform_sensors);

	/* Add the platform specific sensors. */
	if (intf->cb->sensor && intf->cb->sensor->add_sensors) {
		intf->cb->sensor->add_sensors(intf, platform_sensors);
	}

#if 0	/* FIXME: for future reference... */
	/* Add plugin device sensors. */
	add_plugin_device_sensors(intf, sensors);

	/* Add dynamic ata sensors. */
	add_ata_sensors(intf, sensors);
#endif

	return platform_sensors;
}
