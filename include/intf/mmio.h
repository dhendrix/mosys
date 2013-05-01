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

#ifndef INTF_MMIO_H__
#define INTF_MMIO_H__

#include <inttypes.h>

#include "mosys/platform.h"

struct file_backed_range;

struct mmio_intf {
	/* Array of mmio_file_range to use for MMIO access. A
	 * read_file_name and write_file_name of NULL terminate the array. */
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
	 * read  -  read from memory address into buffer
	 *
	 * @intf:     platform interface
	 * @address:  physical address to read from
	 * @length:   length of data to read
	 * @data:     buffer to save data into
	 *
	 * returns length of data read
	 * returns -1 if failed to read
	 */
	int (*read)(struct platform_intf *intf,
		    uint64_t address, int length, void *data);

	/*
	 * write  -  write buffer to memory address
	 *
	 * @intf:     platform interface
	 * @address:  physical address to write into
	 * @length:   length of data to write
	 * @data:     buffer to write into memory
	 *
	 * returns length of data written
	 * returns -1 if failed to write
	 */
	int (*write)(struct platform_intf *intf,
		     uint64_t address, int length, const void *data);

	/*
	 * clear  -  clear range of physical memory
	 *
	 * @intf:       platform interface
	 * @address:    address of physical memory to clear
	 * @length:     length of physical memory to clear
	 *
	 * returns length of memory cleared
	 * returns <0 if failed to clear
	 */
	int (*clear)(struct platform_intf *intf,
		     uint64_t address, int length);

	/*
	 * dump  -  dump range of physical memory
	 *
	 * @intf:     platform interface
	 * @address:  address of memory to dump
	 * @length:   length of data to dump
	 */
	void (*dump)(struct platform_intf *intf,
		     uint64_t address, int length);

	/*
	 * map -  map a piece of physical memory into the address space
	 *
	 * @intf:     platform interface
	 * @flags:    permsissions (O_RDONLY, O_WRONLY, etc)
	 * @address:  address of memory to map
	 * @length:   length of data to map
	 *
	 * return pointer to mapping on success, NULL otherwise.
	 */
	void *(*map)(struct platform_intf *intf, int flags,
		      uint64_t address, unsigned int length);

	/*
	 * unmap -  unmap a piece of physical memory into the address space
	 *
	 * @intf:     platform interface
	 * @mptr:     pointer address is mapped into
	 * @address:  address of memory to unmap
	 * @length:   length of data to unmap
	 *
	 * returns <0 if failure, 0 on success
	 */
	int (*unmap)(struct platform_intf *intf, void *mptr,
		     uint64_t address, unsigned int length);
};

/* Memory-mapped interface */
extern struct mmio_intf mmio_mmap_intf;

/*
 * The following are helpers to make call-sites easier
 */

/*
 * mmio_read  -  read from physical memory
 *
 * @intf:     platform interface
 * @address:  physical address to read from
 * @length:   number of bytes to read
 * @data:     pointer to data buffer
 *
 * returns <0 if failure, 0 on success
 */
static inline int
mmio_read(struct platform_intf *intf, uint64_t address,
          size_t length, void *data)
{
	if (intf->op->mmio->read(intf, address, length, data) != length) {
		return -1;
	}
	return 0;
}

/*
 * mmio_read8  -  read 8 bits from physical memory
 */
static inline int
mmio_read8(struct platform_intf *intf, uint64_t address, uint8_t *data)
{
	return mmio_read(intf, address, sizeof(*data), data);
}

/*
 * mmio_read16  -  read 16 bits from physical memory
 */
static inline int
mmio_read16(struct platform_intf *intf, uint64_t address, uint16_t *data)
{
	return mmio_read(intf, address, sizeof(*data), data);
}

/*
 * mmio_read32  -  read 32 bits from physical memory
 */
static inline int
mmio_read32(struct platform_intf *intf, uint64_t address, uint32_t *data)
{
	return mmio_read(intf, address, sizeof(*data), data);
}

/*
 * mmio_read64  -  read 64 bits from physical memory
 */
static inline int
mmio_read64(struct platform_intf *intf, uint64_t address, uint64_t *data)
{
	return mmio_read(intf, address, sizeof(*data), data);
}

/*
 * mmio_write  -  write from physical memory
 *
 * @intf:     platform interface
 * @address:  physical address to write to
 * @length:   number of bytes to write
 * @data:     pointer to data buffer
 *
 * returns <0 if failure, 0 on success
 */
static inline int
mmio_write(struct platform_intf *intf, uint64_t address,
           size_t length, const void *data)
{
	if (intf->op->mmio->write(intf, address, length, data) != length) {
		return -1;
	}
	return 0;
}

/*
 * mmio_write8  -  write 8 bits to physical memory
 */
static inline int
mmio_write8(struct platform_intf *intf, uint64_t address, const uint8_t data)
{
	return mmio_write(intf, address, sizeof(data), &data);
}

/*
 * mmio_write16  -  write 16 bits to physical memory
 */
static inline int
mmio_write16(struct platform_intf *intf, uint64_t address, const uint16_t data)
{
	return mmio_write(intf, address, sizeof(data), &data);
}

/*
 * mmio_write32  -  write 32 bits to physical memory
 */
static inline int
mmio_write32(struct platform_intf *intf, uint64_t address, const uint32_t data)
{
	return mmio_write(intf, address, sizeof(data), &data);
}

/*
 * mmio_write64  -  write 64 bits to physical memory
 */
static inline int
mmio_write64(struct platform_intf *intf, uint64_t address, const uint64_t data)
{
	return mmio_write(intf, address, sizeof(data), &data);
}

/*
 * mmio_clear  -  clear range of physical memory
 *
 * @intf:       platform interface
 * @address:    address of physical memory to clear
 * @length:     length of physical memory to clear
 *
 * returns <0 if failure, 0 on success
 */
static inline int
mmio_clear(struct platform_intf *intf, uint64_t address, int length)
{
	if (intf->op->mmio->clear(intf, address, length) != length) {
		return -1;
	}
	return 0;
}

/*
 * mmio_dump  -  dump range of physical memory
 *
 * @intf:     platform interface
 * @address:  address of memory to dump
 * @length:   length of data to dump
 */
static inline void
mmio_dump(struct platform_intf *intf, uint64_t address, int length)
{
	intf->op->mmio->dump(intf, address, length);
}

/*
 * mmio_map -  map a piece of physical memory into the address space
 *
 * @intf:     platform interface
 * @flags:    permsissions (O_RDONLY, O_WRONLY, etc)
 * @address:  address of memory to map
 * @length:   length of data to map
 *
 * return pointer to mapping on success, NULL otherwise.
 */
static inline void *
mmio_map(struct platform_intf *intf, int flags,
         uint64_t address, unsigned int length)
{
	return intf->op->mmio->map(intf, flags, address, length);
}

/*
 * mmio_unmap -  unmap a piece of physical memory into the address space
 *
 * @intf:     platform interface
 * @mptr:     pointer address is mapped into
 * @address:  address of memory to unmap
 * @length:   length of data to unmap
 *
 * returns <0 if failure, 0 on success
 */
static inline int
mmio_unmap(struct platform_intf *intf, void *mptr,
           uint64_t address, unsigned int length)
{
	return intf->op->mmio->unmap(intf, mptr, address, length);
}

#endif /* INTF_MMIO_H__ */
