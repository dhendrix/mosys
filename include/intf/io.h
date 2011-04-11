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
 * io.h: Interface for reading and writing to I/O registers
 */

#ifndef INTF_IO_H__
#define INTF_IO_H__

#include <inttypes.h>

#if defined (__i386__) || defined (__x86_64__)
#if defined(__GLIBC__)
#include <sys/io.h>
#endif
#endif

#if defined(__DARWIN__)
/* Header is part of the DirectHW library. */
#include <DirectHW/DirectHW.h>
#define off64_t off_t
#define lseek64 lseek
#endif

enum io_access_width {
	IO_ACCESS_8	= 1,
	IO_ACCESS_16	= 2,
	IO_ACCESS_32	= 4
};

struct platform_intf;
struct io_intf {
	struct file_backed_range *ranges;

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
	 * read  -  Read from IO register
	 *
	 * @intf:     platform interface
	 * @reg:      register
	 * @size:     number of bytes (1/2/4)
	 * @data:     pointer to data buffer
	 *
	 * returns 0 on success, -1 on failure
	 */
	 int (*read)(struct platform_intf *intf, uint16_t reg,
	             enum io_access_width size, void *data);

	/*
	 * write  -  Write to IO register
	 *
	 * @intf:     platform interface
	 * @reg:      register
	 * @size:     number of bytes (1/2/4)
	 * @data:     pointer to data buffer
	 *
	 * returns 0 on success, -1 on failure
	 */
	int (*write)(struct platform_intf *intf, uint16_t reg,
	             enum io_access_width size, void *data);
};

/* IO operations based on system access */
extern struct io_intf io_intf;

/*
 * The following are helpers to make call-sites easier
 */

/*
 * io_read  -  read from an IO port
 *
 * @intf:     platform interface
 * @reg:      port to read from
 * @size:     number of bytes to read (1/2/4)
 * @data:     pointer to data buffer
 *
 * returns <0 if failure, 0 on success
 */
static inline int
io_read(struct platform_intf *intf, uint16_t reg,
        enum io_access_width size, void *data)
{
	return intf->op->io->read(intf, reg, size, data);
}

/*
 * io_read8  -  read 8 bits from an IO port
 */
static inline int
io_read8(struct platform_intf *intf,  uint16_t reg, uint8_t *data)
{
	return io_read(intf, reg, IO_ACCESS_8, data);
}

/*
 * io_read16  -  read 16 bits from an IO port
 */
static inline int
io_read16(struct platform_intf *intf, uint16_t reg, uint16_t *data)
{
	return io_read(intf, reg, IO_ACCESS_16, data);
}

/*
 * io_read32  -  read 32 bits from an IO port
 */
static inline int
io_read32(struct platform_intf *intf, uint16_t reg, uint32_t *data)
{
	return io_read(intf, reg, IO_ACCESS_32, data);
}

/*
 * io_write  -  write from an IO port
 *
 * @intf:     platform interface
 * @reg:      port to read from
 * @size:     number of bytes to read (1/2/4)
 * @data:     pointer to data buffer
 *
 * returns <0 if failure, 0 on success
 */
static inline int
io_write(struct platform_intf *intf, uint16_t reg,
        enum io_access_width size, void *data)
{
	return intf->op->io->write(intf, reg, size, data);
}

/*
 * io_write8  -  write 8 bits to an IO port
 */
static inline int
io_write8(struct platform_intf *intf, uint16_t reg, uint8_t data)
{
	return io_write(intf, reg, IO_ACCESS_8, &data);
}

/*
 * io_write16  -  write 16 bits to an IO port
 */
static inline int
io_write16(struct platform_intf *intf, uint16_t reg, uint16_t data)
{
	return io_write(intf, reg, IO_ACCESS_16, &data);
}

/*
 * io_write32  -  write 32 bits to an IO port
 */
static inline int
io_write32(struct platform_intf *intf, uint16_t reg, uint32_t data)
{
	return io_write(intf, reg, IO_ACCESS_32, &data);
}

#endif /* INTF_IO_H__ */
