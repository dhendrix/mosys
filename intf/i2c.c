/*
 * Copyright (C) 2010 Google Inc.
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
 *
 * i2c.c: I2C bus access via Linux I2C IOCTL interface.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <dirent.h>

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/list.h"
#include "mosys/platform.h"

#include "intf/i2c.h"

#include "lib/math.h"
#include "lib/string.h"

/* these are exposed only to make testing easier */
#define I2C_DEV_ROOT	"/dev"
#define I2C_SYS_ROOT	"/sys/bus/i2c/devices"

/*
 * Block writes need recovery delays for internal processing,
 * according to e.g. the datasheets for Atmel EEPROMs.
 * These are mildly pessimistic defaults.
 */
#define BLOCK_WRITE_DELAY	1000
#define BLOCK_WRITE_RETRIES	32

struct i2c_handle {
	struct i2c_addr addr;
	int fd;
} i2c_handles[I2C_HANDLE_MAX];

static int i2c_handle_num = 0;

/*
 * i2c_open_dev  -  Open connection to I2C slave address
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 *
 * returns handle for open I2C device
 * returns <0 to indicate error
 */
static int i2c_open_dev(struct platform_intf *intf, int bus, int address)
{
	char devf[512];
	int handle, fd;

	if (bus < 0 || bus > 32) {
		lprintf(LOG_NOTICE, "Invalid I2C bus %d\n", bus);
		return -1;
	}

	if (i2c_handle_num >= I2C_HANDLE_MAX) {
		lprintf(LOG_NOTICE, "Out of I2C handles\n");
		return -1;
	}

	for (handle = 0; handle < i2c_handle_num; handle++) {
		if (i2c_handles[handle].addr.bus == bus &&
		    i2c_handles[handle].addr.addr == address &&
		    i2c_handles[handle].fd >= 0)
			return handle;
	}

	snprintf(devf, sizeof(devf), "%s/i2c-%d",
	         intf->op->i2c->dev_root, bus);

	fd = open(devf, O_RDWR);
	if (fd < 0) {
		lperror(LOG_DEBUG, "Unable to open I2C device %s", devf);
		return -1;
	}

#if defined (__linux__)
	if (ioctl(fd, I2C_SLAVE, address) < 0) {
		lperror(LOG_NOTICE, "Unable to set I2C slave address to 0x%02x",
		        address);
		close(fd);
		return -1;
	}
#else
	return -ENOSYS;
#endif

	i2c_handles[i2c_handle_num].addr.bus = bus;
	i2c_handles[i2c_handle_num].addr.addr = address;
	i2c_handles[i2c_handle_num].fd = fd;

	lprintf(LOG_DEBUG, "Opened I2C handle %d to %d-%02x (fd %d)\n",
	        i2c_handle_num, bus, address, fd);

	return i2c_handle_num++;
}

/*
 * i2c_close_dev  -  Close all open I2C handles
 *
 * @intf:       platform interface
 * @handle:     I2C handle to close, -1 for all
 */
static void i2c_close_dev(struct platform_intf *intf)
{
	int i;

	// close all handles
	for (i = 0; i < i2c_handle_num; i++) {
		close(i2c_handles[i].fd);
		i2c_handles[i].fd = -1;
		i2c_handles[i].addr.bus = -1;
		i2c_handles[i].addr.addr = -1;
	}
	i2c_handle_num = 0;

	/* we store these as const in the structure */
	free((char *)intf->op->i2c->sys_root);
	free((char *)intf->op->i2c->dev_root);
}

/*
 * i2c_read_reg  -  Read bytes from a register addressable I2C device
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 * @reg:        I2C register offset
 * @length:     number of bytes to read (1-255)
 * @data:       data buffer
 *
 * returns number of bytes read
 * returns <0 to indicate failure
 */
static int i2c_read_reg(struct platform_intf *intf,
                        int bus, int address, int reg, int length, void *data)
{
	int handle, fd, i;
	int32_t result;
	static int read_words = 1;

	/* limit to 256 bytes at a time */
	if (length < 1 || length > 256) {
		lprintf(LOG_NOTICE, "Invalid I2C read length: %d\n", length);
		return -1;
	}

	lprintf(LOG_DEBUG,
	        "i2c_read_dev: Reading %d bytes from %d-%02x at %02x\n", length,
	        bus, address, reg);

	/* open connection to i2c slave */
	handle = i2c_open_dev(intf, bus, address);
	if (handle < 0)
		return -1;
	fd = i2c_handles[handle].fd;

	memset(data, 0, length);
	i = 0;
	while (i < length) {
		if (read_words && (i < length - 1)) {
			/* Do 2-byte reads whenever possible */
			result = i2c_smbus_read_word_data(fd, reg + i);

                        if (result < 0) {
				if (read_words) {
					/* try again with byte read */
					read_words = 0;
					continue;
				}
				// COV_NF_START
				lperror(LOG_NOTICE,
				        "Failed to read I2C register 0x%02x "
				        "from i2c-%d-%02x", reg + i, bus,
				        address);
				break;
				// COV_NF_END
			}
			memcpy(data + i, &result, 2);
			i = i + 2;
                } else {
			/* Do an 1-byte read otherwise*/
			result = i2c_smbus_read_byte_data(fd, reg + i);

			if (result < 0) {
				lperror(LOG_NOTICE,
				        "Failed to read I2C register 0x%02x "
				        "from i2c-%d-%02x", reg + i, bus,
				        address);
				break;
			}
			memcpy(data + i, &result, 1);
			i++;
		}
	}

	return i;
}

/*
 * i2c_read16_dev  -  Read bytes from I2C device using 16-bit register offset
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 * @reg:        I2C register offset (16-bit)
 * @length:     number of bytes to read (1-255)
 * @data:       data buffer
 *
 * returns number of bytes read
 * returns <0 to indicate failure
 *
 * We can't actually use i2c_smbus_read_block_data() because the driver
 * doesn't know how to do the 2-byte address write. So we do the best we can
 * by performing the address write once, then calling i2c_smbus_read_byte()
 * repeatedly to keep the overhead to a minimum.
 */
static int i2c_read16_dev(struct platform_intf *intf,
                          int bus, int address, int reg, int length, void *data)
{
	uint8_t hi, lo;
	int fd, handle, i;
	int32_t result;
	uint8_t *dp = data;

	memset(data, 0, length);

	lprintf(LOG_DEBUG,
	        "i2c_read16_dev: Reading %d bytes from %d-%02x at %02x\n",
	        length, bus, address, reg);

	// open connection to i2c slave
	handle = i2c_open_dev(intf, bus, address);
	if (handle < 0)
		return -1;
	fd = i2c_handles[handle].fd;

	// Write eeprom offset
	hi = reg >> 8;
	lo = reg & 0xff;
	result = i2c_smbus_write_byte_data(fd, hi, lo);
	if (result < 0) {
		lperror(LOG_NOTICE,
		        "Failed to write I2C register 0x%04x from i2c-%d-%02x",
		        reg, bus, address);
		return -1;
	}

	// Read the block
	for (i = 0; i < length; ++i) {
		result = i2c_smbus_read_byte(fd);
		if (result < 0) {
			lperror(LOG_NOTICE,
			        "Failed to read I2C register 0x%04x "
			        "from i2c-%d-%02x", reg + i, bus, address);
			return -1;
		}
		dp[i] = result;
	}

	return i;
}

/*
 * i2c_read_raw  -  read byte(s) from device without register addressing
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 * @length:	number of bytes to read
 * @data:       data buffer
 *
 * returns number of bytes read
 * returns <0 to indicate failure
 */
static int i2c_read_raw(struct platform_intf *intf,
                        int bus, int address, int length, void *data)
{
	int result, count;
	int fd, handle;
	uint8_t *dp = data;

	lprintf(LOG_DEBUG,
	        "%s: Reading byte from %d-%02x\n",
	        __func__, bus, address);

	/* open connection to i2c slave */
	handle = i2c_open_dev(intf, bus, address);
	if (handle < 0)
		return -1;
	fd = i2c_handles[handle].fd;

	for (count = 0; count < length; count++) {
		/* read byte */
		result = i2c_smbus_read_byte(fd);
		if (result < 0) {
			lperror(LOG_NOTICE,
			        "%s: Failed to read from from i2c-%d-%02x",
			        __func__, bus, address);
			break;
		}

		dp[count] = result;
	}

	return count;
}

/*
 * i2c_write_reg  -  Write bytes to I2C slave address
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 * @reg:        I2C register offset
 * @length:     number of bytes to read (1-255)
 * @data:       data buffer
 *
 * returns number of bytes written
 * returns <0 to indicate error
 */
static int i2c_write_reg(struct platform_intf *intf,
                         int bus,
                         int address, int reg, int length, const void *data)
{
	int handle, fd, i;
	int32_t result;
	const uint8_t *data_ptr = data;

	if (length < 1 || length > 256) {
		lprintf(LOG_NOTICE, "Invalid I2C write length: %d\n", length);
		return -1;
	}

	lprintf(LOG_DEBUG,
	        "i2c_write_dev: Writing %d bytes to %d-%02x at %02x\n",
	        length, bus, address, reg);

	/* open connection to i2c slave */
	handle = i2c_open_dev(intf, bus, address);
	if (handle < 0)
		return -1;
	fd = i2c_handles[handle].fd;

	for (i = 0; i < length; i++) {
		/* write one byte at a time */
		result = i2c_smbus_write_byte_data(fd, reg + i, data_ptr[i]);
		if (result < 0) {
			lperror(LOG_NOTICE,
			        "Failed to write I2C register 0x%02x"
			        " from i2c-%d-%02x", reg, bus, address);
			i = -1;
			break;
		}
	}

	return i;
}

/*
 * i2c_write16_buf  -  Write a buffer to I2C slave address using 16-bit offset
 *
 * @handle:     I2C device handle
 * @reg:        I2C register offset (16-bit)
 * @dp:         data buffer
 * @len:        number of bytes to write (1-32)
 *
 * returns number of bytes written
 * returns <0 to indicate error
 *
 * The LSByte of the 16-bit register offset is sent as the first byte of
 * the transfer buffer. Note the driver won't let us transfer more than
 * 32 bytes, and since we're using one data byte as the low byte of the
 * offset address, we have to perform the write in 31-byte chunks. Ugh.
 */
static int i2c_write16_buf(int handle, int reg, const uint8_t *dp, int len)
{
	uint8_t buf[32];
	int32_t result;
	int fd = i2c_handles[handle].fd;
	int bus = i2c_handles[handle].addr.bus;
	int address = i2c_handles[handle].addr.addr;
	int i;

	memset(buf, 0, 32);
	buf[0] = reg & 0xff;	// LSByte of register offset
	len = __min(len, 31);
	memcpy(&buf[1], dp, len);

	result = i2c_smbus_write_i2c_block_data(fd, reg >> 8, len+1, buf);

	if (result < 0) {
		lperror(LOG_NOTICE,
		        "Failed to write %d bytes "
		        "to I2C register 0x%04x from i2c-%d-%02x",
		        len, reg, bus, address);
		return result;
	}

	// Spin waiting for device to recover.
	for (i = 0; i < BLOCK_WRITE_RETRIES; ++i) {
		usleep(BLOCK_WRITE_DELAY);
#if defined(__linux__)
		if (ioctl(fd, I2C_SLAVE, address) == 0)
			return result;
#else
		return -ENOSYS;
#endif
	}
	lperror(LOG_NOTICE,
	        "i2c-%d-%02x didn't recover in %d usec\n",
	        bus, address,
	        BLOCK_WRITE_RETRIES * BLOCK_WRITE_DELAY);
	return result;
}

/*
 * i2c_write16_dev  -  Write bytes to I2C slave address using 16-bit reg offset
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 * @reg:        I2C register offset (16-bit)
 * @length:     number of bytes to read (1-255)
 * @data:       data buffer
 *
 * returns number of bytes written
 * returns <0 to indicate error
 *
 */
static int i2c_write16_dev(struct platform_intf *intf,
                           int bus, int address, int reg,
                           int length, const void *data)
{
	int handle, written = 0;
	const uint8_t *data_ptr = (uint8_t *)data;
	int buf_len, count;

	// Open connection to this address
	handle = i2c_open_dev(intf, bus, address);
	if (handle < 0)
		return -1;

	if (length == 0) {
		/* useful for setting internal address counters in devices */
		written = i2c_write16_buf(handle, reg, NULL, 0);
	}

	while (length) {
		buf_len = __min(length, 31);
		count = i2c_write16_buf(handle, reg, data_ptr, buf_len);
		if (count != buf_len)
			return written;
		length -= count;
		written += count;
		data_ptr += count;
		reg += count;

		if (length == 0)
			return written;

		buf_len = 1;
		count = i2c_write16_buf(handle, reg, data_ptr, buf_len);
		if (count != buf_len)
			return written;
		length -= buf_len;
		written += buf_len;
		data_ptr += buf_len;
		reg += buf_len;
	}
	return written;
}

/*
 * i2c_write_raw  -  write byte(s) to device without register addressing
 *
 * @intf:       platform interface
 * @bus:        I2C bus/adapter
 * @address:    I2C slave address
 * @data:       data buffer
 *
 * returns number of bytes written
 * returns <0 to indicate failure
 */
static int i2c_write_raw(struct platform_intf *intf,
                         int bus, int address, int length, void *data)
{
	int handle, fd, count;
	int32_t result;
	const uint8_t *data_ptr = data;

	lprintf(LOG_DEBUG,
	        "%s: Writing byte to %d-%02x\n",
	        __func__, bus, address);

	/* open connection to i2c slave */
	handle = i2c_open_dev(intf, bus, address);
	if (handle < 0)
		return -1;
	fd = i2c_handles[handle].fd;

	for (count = 0; count < length; count++) {
		/* write byte */
		result = i2c_smbus_write_byte(fd, data_ptr[count]);
		if (result < 0) {
			lperror(LOG_NOTICE,
			        "Failed to write byte to i2c-%d-%02x",
			        bus, address);
			break;
		}
  	}

	return count;
}

/*
 * i2c_find_driver - Determine if i2c driver is loaded by scanning
 * /proc/modules.
 *
 * @intf:	Platform interface
 * @module: 	The name of the module to search for
 *
 * returns 0 if the module is not found or if /proc/modules does not exist
 * returns > 0 if the module is found
 */
static int i2c_find_driver(struct platform_intf *intf, const char *module)
{
	char *path;
	char s[80];
	FILE *fp;
	int len = strlen(module);
	int ret = 0;

	path = format_string("%s/proc/modules", mosys_get_root_prefix());
	fp = fopen(path, "r");
	free(path);
	if (fp == NULL)
		return 0;
	len = __min(len, 80);
	while (fgets(s, 80, fp) != NULL) {
		if (!strncmp(s, module, len)) {
			ret = 1;
			break;
		}
	}

	fclose(fp);
	return ret;
}

/*
 * i2c_match_bus_name  -  Look for bus name
 *
 * @intf:	platform interface
 * @name:	bus name
 * @bus:	bus number
 *
 * returns 1 if bus name does match
 * returns 0 if bus name does not match
 */
static int i2c_match_bus_name(struct platform_intf *intf,
                              int bus, const char *name)
{
	char *path;
	char bus_name[64];
	int fd;
	int len = strlen(name);
	int ret = 0;

	path = format_string("%s/sys/class/i2c-dev/i2c-%d/name",
	                     mosys_get_root_prefix(), bus);

	fd = open(path, O_RDONLY);
	free(path);
	if (fd < 0)
		return 0;
	len = __min(len, 64);
	memset(bus_name, 0, len);
	if (read(fd, bus_name, len) == len) {
		if (strncmp(bus_name, name, len) == 0)
			ret = 1;
	}

	close(fd);
	return ret;
}

/*
 * i2c_find_dir - Find a /sys directory or directories for a device's
 * description as printed in the /sys/bus/i2c/devices/<bus-addr>/name file.
 * All matching descriptions are inserted into a linked list.
 *
 * @intf:	Platform interface
 * @name: 	The device name to search for, eg "eeprom"
 *
 * returns the head of a linked list of matching devices if found.
 * returns NULL if no device found or if an error has occured.
 */
static struct ll_node *i2c_find_dir(struct platform_intf *intf,
                                    const char *name)
{
	char *path;
	char s[32];
	FILE *fp;
	DIR *dp;
	struct dirent *d;
	struct ll_node *head = NULL;
	int bus, addr;
	int len = strlen(name);

	if (!(dp = opendir(intf->op->i2c->sys_root))) {
		lprintf(LOG_ERR, "Failed to open %s\n",
		        intf->op->i2c->sys_root);
		return NULL;
	}

	while ((d = readdir(dp))) {
		/* Scan for entries in the form of 0-0000 and filter out
		 * irrelevent entries like '.' and ".." */
		if (!(sscanf(d->d_name, "%u-%x", &bus, &addr)))
			continue;

		/* Is this the type of device we want? */
		path = format_string("%s/%u-%04x/name",
		                     intf->op->i2c->sys_root, bus, addr);
		fp = fopen(path, "r");
		if (fp) {
			fgets(s, sizeof(s), fp);
			fclose(fp);
			if (!strncmp(s, name, len)) { /* Found one! */
				struct i2c_data *n;

				n = mosys_malloc(sizeof(struct i2c_data));

				/* Fill out the data structure */
				n->addr.bus = bus;
				n->addr.addr = addr;
				n->dir = format_string("%s/%u-%04x",
				                       intf->op->i2c->sys_root,
				                       bus, addr);

				head = list_insert_before(head, (void *)n);
			}
		} else {
			lprintf(LOG_NOTICE,                   // COV_NF_LINE
			        "Failed to open %s\n", path);
		}
		free(path);
	}

	closedir(dp);
	return head;
}

static int i2c_setup_dev(struct platform_intf *intf)
{
	intf->op->i2c->sys_root
	        = format_string("%s/%s", mosys_get_root_prefix(), I2C_SYS_ROOT);
	intf->op->i2c->dev_root
	        = format_string("%s/%s", mosys_get_root_prefix(), I2C_DEV_ROOT);
	return 0;
}

/* I2C operations based on Linux /dev interface */
struct i2c_intf i2c_dev_intf = {
	.setup  	= i2c_setup_dev,
	.destroy	= i2c_close_dev,
	.read_reg	= i2c_read_reg,
	.write_reg	= i2c_write_reg,
	.read16		= i2c_read16_dev,
	.write16	= i2c_write16_dev,
	.read_raw	= i2c_read_raw,
	.write_raw	= i2c_write_raw,
	.find_driver	= i2c_find_driver,
	.find_dir	= i2c_find_dir,
	.match_bus	= i2c_match_bus_name,
};
