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
 * I2C (Inter-Integrated Circuit) is a multi-master serial computer bus
 * invented by Philips that is used to attach low-speed peripherals to 
 * a motherboard, embedded system, or cellphone.
 */

#ifndef INTF_I2C_H__
#define INTF_I2C_H__

#include <inttypes.h>
#include "mosys/list.h"

#define I2C_HANDLE_MAX		64

struct platform_intf;
struct i2c_intf {
	const char *sys_root;
	const char *dev_root;

	/*
	 * setup  -  prepare interface
	 *
	 * @intf:       platform interface
	 *
	 * returns 0 to indicate success
	 * returns <0 to indicate failure
	 */
	int (*setup)(struct platform_intf *intf);

	/*
	 * destroy  -  teardown interface
	 *
	 * @intf:       platform interface
	 */
	void (*destroy)(struct platform_intf *intf);

	/*
	 * read  -  Read bytes from I2C device
	 *
	 * @intf:       platform interface
	 * @bus:        I2C bus/adapter
	 * @address:    I2C slave address
	 * @reg:        I2C register offset
	 * @length:     number of bytes to read (1-255)
	 * @data:       data buffer
	 *
	 * returns number of bytes read
	 * returns <0 to indicate error
	 */
	int (*read_reg)(struct platform_intf *intf,
			int bus, int address, int reg,
			int length, void *data);

	/*
	 * write  -  Write bytes to I2C device
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
	int (*write_reg)(struct platform_intf *intf,
			 int bus, int address, int reg,
			 int length, const void *data);

	/*
	 * read16 -  Read bytes from I2C device using 16-bit address
	 *
	 * @intf:       platform interface
	 * @bus:        I2C bus/adapter
	 * @address:    I2C slave address
	 * @reg:        I2C register offset (16-bit)
	 * @length:     number of bytes to read (1-255)
	 * @data:       data buffer
	 *
	 * returns number of bytes read
	 * returns <0 to indicate error
	 */
	int (*read16)(struct platform_intf *intf,
		      int bus, int address, int reg,
		      int length, void *data);

	/*
	 * write16 -  Write bytes to I2C device using 16-bit address
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
	 */
	int (*write16)(struct platform_intf *intf,
		       int bus, int address, int reg,
		       int length, const void *data);

	/*
	 * read_raw  -  read byte(s) from device without register addressing
	 *
	 * @intf:       platform interface
	 * @bus:        I2C bus/adapter
	 * @address:    I2C slave address
	 * @length:	number of bytes to read (1-256)
	 * @data:       data buffer
	 *
	 * returns number of bytes read
	 * returns <0 to indicate failure
	 */
	int (*read_raw)(struct platform_intf *intf,
			int bus, int address, int length, void *data);

	/*
	 * write_raw  -  write byte(s) to device without register addressing
	 *
	 * @intf:       platform interface
	 * @bus:        I2C bus/adapter
	 * @address:    I2C slave address
	 * @length:	number of bytes to write (1-256)
	 * @data:       data buffer
	 *
	 * returns number of bytes written
	 * returns <0 to indicate failure
	 */
	int (*write_raw)(struct platform_intf *intf,
			 int bus, int address, int length, void *data);

	/*
	 * find_driver  -  Find I2C driver
	 *
	 * @intf:	platform interface
	 * @name:	i2c driver name
	 *
	 * returns 0 if the module is not found
	 * returns 1 if the module is found
	 */
	int (*find_driver)(struct platform_intf *intf,
			   const char *name);

	/*
	 * find_dir  -  Find I2C directory for device
	 *
	 * @intf:	platform interface
	 * @name:	i2c driver name
	 *
	 * returns the head of a linked list of matching devices
	 * returns NULL if no device found
	 */
	struct ll_node *(*find_dir)(struct platform_intf *intf,
				    const char *name);

	/*
	 * match_bus  -  Look for bus name
	 *
	 * @intf:	platform interface
	 * @name:	bus name
	 * @bus:	bus number
	 *
	 * returns 1 if bus name does match
	 * returns 0 if bus name does not match
	 */
	int (*match_bus)(struct platform_intf *intf,
			 int bus, const char *name);
};

/* I2C operations for Linux /dev interface */
extern struct i2c_intf i2c_dev_intf;

struct i2c_addr {
	int bus;
	int addr;
};

/* Data for storing /sys i2c information */
struct i2c_data {
	struct i2c_addr addr;
	char *dir;
};

#endif /* INTF_I2C_H__ */
