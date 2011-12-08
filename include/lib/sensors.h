/*
 * Copyright 2011, Google Inc.
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

#ifndef MOSYS_LIB_SENSORS_H__
#define MOSYS_LIB_SENSORS_H__

#include <inttypes.h>
#include <unistd.h>
#include <valstr.h>

/*
 * Identify the various types of sensors that we know about.  These values
 * are used in bitmasks.
 */
typedef enum sensor_type {
	SENSOR_TYPE_THERMAL_DEGREES  = 0x00000001,
	SENSOR_TYPE_THERMAL_MARGIN   = 0x00000002,
	SENSOR_TYPE_THERMAL_TCONTROL = 0x00000004,
	SENSOR_TYPE_THERMAL          = SENSOR_TYPE_THERMAL_DEGREES
	                             | SENSOR_TYPE_THERMAL_MARGIN
	                             | SENSOR_TYPE_THERMAL_TCONTROL,
	SENSOR_TYPE_VOLTAGE          = 0x00000008,
	SENSOR_TYPE_FANTACH          = 0x00000010,
	SENSOR_TYPE_CURRENT          = 0x00000020,
	SENSOR_TYPE_POWER            = 0x00000040,
	SENSOR_TYPE_ALL              = 0xffffffff,
} sensor_type;

/* A set of sensor types. */
typedef unsigned sensor_type_mask;

/* Sensor flags. */
typedef enum sensor_flag {
	SENSOR_FLAG_VERBOSE_ONLY = 0x01,
} sensor_flag;

/* A set of sensor flags. */
typedef uint8_t sensor_flag_mask;

/* Forward declaration. */
struct sensor;
struct platform_intf;

/* info that gets passed between read functions and callers */
#define SENSOR_MODE_MANUAL	(1 << 0)
#define SENSOR_MODE_AUTO	(1 << 1)

extern const struct valstr sensor_modes[];

struct sensor_reading {
	double value;		/* assume all readings are doubles for now... */
	unsigned int mode;
};

/* A I2C sensor address. */
struct sensor_addr_i2c {
	int bus;
	int addr;
	int reg;
};

union sensor_addr {
	int sp;				/* Sensorpath location */
	int cpu;			/* CPU number */
	int dimm;			/* DIMM number */
	int sysfs_num;			/* Sysfs number (e.g. 0 for temp0) */
	int reg;			/* Register offset (e.g. SIO index) */
	struct sensor_addr_i2c i2c;	/* I2C address tuple */
};

/*
 * sensor_read_function  -  get sensor reading
 *
 * @intf:	platform interface
 * @sensor:	sensor to read
 * @reading:	pointer to variable to store reading
 *
 * returns 0 if successful
 * returns <0 to indicate error
 */
typedef int (*sensor_read_function)(struct platform_intf *intf,
                                    struct sensor *sensor,
                                    struct sensor_reading *reading);

/* A mosys sensor descriptor. */
struct sensor {
	const char *name;
	sensor_type type;
	union sensor_addr addr;
	sensor_read_function read;
	sensor_flag_mask flags;
	void *priv;
};

/*
 * Sensor Element Routines
 */

/* new_sensor - allocates a new sensor structure.
 *
 * This function creates a new sensor structure, assigning all of the requested
 * fields and copying the "name" field to the new instance.
 *
 * @name:     name of the new sensor
 * @type:     type of the new sensor
 * @addr:     address of the new sensor
 * @read:     function for reading the new sensor
 * @flags:    flags for the new sensor
 * @priv:     free-form private data related to the new sensor (Note that this
 *            is NOT copied).
 *
 * The memory allocated by this function is owned by the caller.
 */
struct sensor *new_sensor(const char *name, sensor_type type,
                          union sensor_addr addr, sensor_read_function read,
                          sensor_flag_mask flags, void *priv);

/* free_sensor - free the allocated sensor
 *
 * This function frees the name pointed to by the structure and the structure
 * itself. It makes no attempt to manage the "priv" data member.
 */
void free_sensor(struct sensor *sensor);

/*
 * Sensor Array Routines
 */
struct sensor_array;

/* new_sensor_array - allocate a sensor_array structure with default size
 *
 * returns an allocated sensor_array
 */
extern struct sensor_array *new_sensor_array(void);

/* free_sensor_array - free the allocated sensor_array
 *
 * @array:  sensor_array to free
 */
extern void free_sensor_array(struct sensor_array *array);

/* free_sensor_array_ptr - free the allocated sensor_array pointed to by ptr
 *
 * @ptr:  pointer to sensor_array to free and NULL out
 */
extern void free_sensor_array_ptr(struct sensor_array **ptr);

/* add_sensor - add a sensor to the array
 *
 * @array:  sensor_array to add sensor to
 * @sensor:  sensor to add
 *
 * returns 0 on success, < 0 if failure
 */
extern int add_sensor(struct sensor_array *array, struct sensor *sensor);

/* add_sensors - add a group of sensors to the array
 *
 * @array:  sensor_array to add sensor to
 * @sensors:  sensors to add
 * @num:  number of sensors to add
 *
 * returns number of sensors added
 */
extern size_t add_sensors(struct sensor_array *array,
                          struct sensor *sensors, size_t num);

/* num_sensors - return the number of sensors in given array
 *
 * @array:  sensor_array to obtain number of sensors
 *
 * returns number of sensors in array, -1 if error
 */
extern size_t num_sensors(struct sensor_array *array);

/* get_sensor - return the requested sensor from the given array
 *
 * @array:  sensor_array to obtain number of sensors
 * @entry:  entry number of the sensor within the array (0 based)
 *
 * returns requested sensor if successful, NULL if error
 */
extern struct sensor *get_sensor(struct sensor_array *array, size_t entry);

/* get_sensor_by_name_and_type - return the requested sensor
 *         from the given array, if name and type matched
 *
 * @array:  sensor_array to obtain number of sensors
 * @name:   sensor's name
 * @type:   enum sensor_type
 *
 * returns requested sensor if successful, NULL if error
 */
extern struct sensor *get_sensor_by_name_and_type(struct sensor_array *array,
                                                  const char *name,
                                                  sensor_type type);
/*
 * get_platform_sensors - obtain the platform's sensor array
 *
 * @intf: platform interface
 *
 * Always returns the platform's array of sensors.
 */
extern struct sensor_array *get_platform_sensors(struct platform_intf *intf);

#endif  /* MOSYS_LIB_SENSORS_H__ */
