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
