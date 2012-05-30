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
 *
 * cypress_apa.c: Query info from Cypress trackpad. If the cyapa kernel driver
 * is loaded, the information will come from ioctl. Otherwise, it will come from
 * raw I2C access.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "intf/i2c.h"

#include "lib/file.h"
#include "lib/string_builder.h"

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "drivers/cypress/cypress_apa.h"

static int cyapa_read_sysfs_value(struct platform_intf *intf,
                                  const char *path, char **buf, size_t max_len)
{
	int fd, len, rc = 0;
	char *raw_value = mosys_zalloc(max_len);

	fd = file_open(path, FILE_READ);
	if (fd < 0) {
		rc = -1;
		goto cyapa_read_sysfs_value_exit_1;
	}

	len = read(fd, raw_value, max_len);
	if (len < 0) {
		lperror(LOG_DEBUG, "%s: Unable to read value from %s",
		        __func__, path);
		rc = -1;
		goto cyapa_read_sysfs_value_exit_0;
	}

	*buf = mosys_zalloc(len);
	snprintf(*buf, strlen(raw_value), "%s", raw_value);

cyapa_read_sysfs_value_exit_1:
	close(fd);
cyapa_read_sysfs_value_exit_0:
	free(raw_value);
	return rc;
}

static int cyapa_get_firmware_version_sysfs(struct platform_intf *intf,
                                            uint8_t bus, uint8_t addr,
                                            char **buf)
{
	int ret = 0;
	struct string_builder *sb = new_string_builder();

	string_builder_sprintf(sb, "%s/i2c-%x/%x-%04x/%s",
	                       intf->op->i2c->sys_root, bus, bus, addr,
	                       CYAPA_SYSFS_FIRMWARE_VERSION);

	ret = cyapa_read_sysfs_value(intf, string_builder_get_string(sb), buf, 8);
	free_string_builder(sb);
	return ret;
}

static int cyapa_get_hardware_version_sysfs(struct platform_intf *intf,
                                            uint8_t bus, uint8_t addr,
                                            char **buf)
{
	int ret = 0;
	struct string_builder *sb = new_string_builder();

	string_builder_sprintf(sb, "%s/i2c-%x/%x-%04x/%s",
	                       intf->op->i2c->sys_root, bus, bus, addr,
	                       CYAPA_SYSFS_HARDWARE_VERSION);

	ret = cyapa_read_sysfs_value(intf, string_builder_get_string(sb), buf, 8);
	free_string_builder(sb);
	return ret;
}

static int cyapa_get_product_id_sysfs(struct platform_intf *intf,
                                      uint8_t bus, uint8_t addr, char **buf)
{
	int ret = 0;
	struct string_builder *sb = new_string_builder();

	string_builder_sprintf(sb, "%s/i2c-%x/%x-%04x/%s",
	                       intf->op->i2c->sys_root, bus, bus, addr,
	                       CYAPA_SYSFS_PRODUCT_ID);

	ret = cyapa_read_sysfs_value(intf, string_builder_get_string(sb), buf, 32);
	free_string_builder(sb);
	return ret;
}

static int cyapa_get_firmware_version_i2c(struct platform_intf *intf,
                                          uint8_t bus, uint8_t addr, char **buf)
{
	uint8_t major, minor;
	size_t max_len = 8;

	if (intf->op->i2c->smbus_read_reg(intf, bus, addr,
					  CYAPA_REG_FIRMWARE_MAJOR,
					  1, &major) != 1)
		return -1;

	if (intf->op->i2c->smbus_read_reg(intf, bus, addr,
					  CYAPA_REG_FIRMWARE_MINOR,
					  1, &minor) != 1)
		return -1;

	*buf = mosys_zalloc(max_len);
	snprintf(*buf, max_len, "%u.%u", major, minor);
	return 0;
}

static int cyapa_get_hardware_version_i2c(struct platform_intf *intf,
                                          uint8_t bus, uint8_t addr, char **buf)
{
	uint8_t major, minor;
	size_t max_len = 8;

	if (intf->op->i2c->smbus_read_reg(intf, bus, addr,
					  CYAPA_REG_HARDWARE_MAJOR,
					  1, &major) != 1)
		return -1;

	if (intf->op->i2c->smbus_read_reg(intf, bus, addr,
					  CYAPA_REG_HARDWARE_MINOR,
					  1, &minor) != 1)
		return -1;

	*buf = mosys_zalloc(max_len);
	snprintf(*buf, max_len, "%u.%u", major, minor);
	return 0;
}

static int cyapa_get_product_id_i2c(struct platform_intf *intf,
                                    uint8_t bus, uint8_t addr, char **buf)
{
	uint8_t i;

	*buf = mosys_zalloc(CYAPA_REG_PRODUCT_ID_LEN + 1);
	for (i = 0; i < CYAPA_REG_PRODUCT_ID_LEN; i++) {
		if (intf->op->i2c->smbus_read_reg(intf, bus, addr,
						  CYAPA_REG_PRODUCT_ID + i,
						  1, *buf + i) != 1)
		return -1;
	}

	return 0;
}

int cyapa_get_firmware_version(struct platform_intf *intf, uint8_t bus,
                               uint8_t addr, char **buf)
{
	int rc = 0;

	rc = cyapa_get_firmware_version_sysfs(intf, bus, addr, buf);
	if (rc != 0) {
		lprintf(LOG_DEBUG, "%s: Using I2C fallback\n", __func__);
		rc = cyapa_get_firmware_version_i2c(intf, bus, addr, buf);
	}
	return rc;
}

int cyapa_get_hardware_version(struct platform_intf *intf, uint8_t bus,
                               uint8_t addr, char **buf)
{
	int rc = 0;

	rc = cyapa_get_hardware_version_sysfs(intf, bus, addr, buf);
	if (rc != 0) {
		lprintf(LOG_DEBUG, "%s: Using I2C fallback\n", __func__);
		rc = cyapa_get_hardware_version_i2c(intf, bus, addr, buf);
	}
	return rc;
}

int cyapa_get_product_id(struct platform_intf *intf, uint8_t bus,
                         uint8_t addr, char **buf)
{
	int rc = 0;

	rc = cyapa_get_product_id_sysfs(intf, bus, addr, buf);
	if (rc != 0) {
		lprintf(LOG_DEBUG, "%s: Using I2C fallback\n", __func__);
		rc = cyapa_get_product_id_i2c(intf, bus, addr, buf);
	}
	return rc;


}
