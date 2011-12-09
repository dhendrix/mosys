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
 *
 * dts.c: Functions for handling integrated digital thermal sensors
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosys/globals.h"
#include "mosys/log.h"

#include "lib/file.h"
#include "lib/sensors.h"

static int sysfs_read_coretemp(struct platform_intf *intf,
                               struct sensor *sensor,
                               struct sensor_reading *reading)
{
	char coretemp_dir[512];
	char path[512];
	char label[32];
	char input[8];	/* allow up to 7-digits + terminator */
	int fd, len = -1, rc = -1;
	int sensor_num;

	if (!sensor || !reading)
		return -1;

	sensor_num = sensor->addr.dts.sensor_num;

	sprintf(coretemp_dir, "%s/sys/bus/platform/devices/coretemp.%d",
	        mosys_get_root_prefix(), sensor->addr.dts.package);

	sprintf(path, "%s/temp%d_label", coretemp_dir, sensor_num);
	fd = file_open(path, FILE_READ);
	if (fd < 0) {
		lperror(LOG_DEBUG, "Cannot open %s", path);
		goto coretemp_read_exit_1;
	}

	len = read(fd, label, sizeof(label));
	if (len < 0) {
		lperror(LOG_DEBUG, "Cannot read sensor %d label", sensor_num);
		goto coretemp_read_exit_2;
	}
	label[len - 1] = '\0';

	close(fd);

	if (strncmp(label, sensor->name, strlen(sensor->name))) {
		lprintf(LOG_DEBUG, "%s: \"%s\" != \"%s\"\n",
		        __func__, label, sensor->name);
		goto coretemp_read_exit_1;
	}

	sprintf(path, "%s/temp%d_input", coretemp_dir, sensor_num);
	fd = file_open(path, FILE_READ);
	if (fd < 0) {
		lperror(LOG_DEBUG, "Cannot open %s", path);
		goto coretemp_read_exit_1;
	}

	label[len - 1] = '\0';
	if (len < 0) {
		lperror(LOG_DEBUG, "Cannot read sensor %d temp", sensor_num);
		goto coretemp_read_exit_2;
	}
	len = read(fd, input, sizeof(input));

	/* value is presented in millidegrees Celsius */
	errno = 0;
	reading->value = strtod(input, NULL) / 1000;
	if (!errno)
		rc = 0;

coretemp_read_exit_2:
	close(fd);
coretemp_read_exit_1:
	return rc;
}

int dts_read(struct platform_intf *intf,
                  struct sensor *sensor, struct sensor_reading *reading)
{
	/* FIXME: Add direct method */
	return sysfs_read_coretemp(intf, sensor, reading);
}
