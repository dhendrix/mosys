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

#include <stdlib.h>
#include <unistd.h>

#include "mosys/globals.h"
#include "mosys/log.h"

#include "lib/acpi.h"
#include "lib/file.h"
#include "lib/sensors.h"

int acpi_read_temp(struct platform_intf *intf,
                   struct sensor *sensor, struct sensor_reading *reading)
{
	char path[512];
	char buf[8];		/* allow up to 7-digits + terminator */
	int fd, len = -1;

	sprintf(path, "%s/sys/class/thermal/thermal_zone%d/temp",
	        mosys_get_root_prefix(), sensor->addr.sysfs_num);

	fd = file_open(path, FILE_READ);
	if (fd < 0)
		return -1;

	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		lprintf(LOG_DEBUG, "%s: failed to read temperature from %s\n",
		        __func__, path);
	}

	/* thermal_zone value is multiplied by 1000 */
	reading->value = strtod(buf, NULL) / 1000;
	close(fd);
	return 0;
}
