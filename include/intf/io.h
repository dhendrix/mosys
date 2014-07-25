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
 *
 * io.h: Interface for reading and writing to I/O registers
 */

#ifndef INTF_IO_H__
#define INTF_IO_H__

#include <inttypes.h>

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
