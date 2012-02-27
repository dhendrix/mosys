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
 * PCI (Peripheral Component Interconnect) Interface
 */

#ifndef MOSYS_INTF_PCI_H__
#define MOSYS_INTF_PCI_H__

#include <inttypes.h>

struct platform_intf;

typedef int (*pci_callback_t)(struct platform_intf *intf,
			      int bus, int dev, int func, void *data);

struct pci_intf {
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
	 * read  -  Read from PCI configuration space
	 *
	 * @intf:     platform interface
	 * @bus:      pci bus
	 * @dev:      pci device
	 * @func:     pci function
	 * @reg:      configuration register
	 * @length:   number of bytes to read
	 * @data:     buffer to read data into
	 *
	 * returns number of bytes read
	 * returns <0 to indicate failure
	 */
	int (*read)(struct platform_intf *intf,
		    int bus, int dev, int func, int reg,
		    int length, void *data);

	/*
	 * write  -  Write to PCI configuration space
	 *
	 * @intf:     platform interface
	 * @bus:      pci bus
	 * @dev:      pci device
	 * @func:     pci function
	 * @reg:      configuration register
	 * @length:   number of bytes to read
	 * @data:     buffer to read data into
	 *
	 * returns number of bytes written
	 * returns <0 to indicate failure
	 */
	int (*write)(struct platform_intf *intf,
		     int bus, int dev, int func, int reg,
		     int length, const void *data);

	/*
	 * foreach  -  Execute function on all PCI devices
	 *
	 * @intf:	platform interface
	 * @cb:		pci callback
	 * @data:	data supplied to callback
	 *
	 * Supplied PCI callback will cause bus enumeration to stop if
	 * it returns non-zero value.
	 *
	 * returns 0 to indicate success, failure otherwise.
	 */
	int (*foreach)(struct platform_intf *intf,
		       pci_callback_t cb, void *data);

	/*
	 * foreach_in_bus  -  Execute function on all PCI devices on a bus
	 *
	 * @intf:	platform interface
	 * @bus:	target pci bus
	 * @cb:		pci callback
	 * @data:	data supplied to callback
	 *
	 * Supplied PCI callback will cause bus enumeration to stop if
	 * it returns non-zero value.
	 *
	 * returns 0 to indicate success, failure otherwise.
	 */
	int (*foreach_in_bus)(struct platform_intf *intf, int bus,
                              pci_callback_t cb, void *data);
};

struct pci_addr {
	unsigned int bus;
	unsigned int dev;
	unsigned int func;
};

/* PCI operations based on /proc directory */
extern struct pci_intf pci_file_intf;

/*
 * The following are helpers to make call-sites easier
 */

/*
 * pci_read  -  read from PCI config space
 *
 * @intf:     platform interface
 * @bus:      PCI bus
 * @dev:      PCI device
 * @func:     PCI function
 * @reg:      configuration register
 * @length:   number of bytes to read
 * @data:     buffer to read data into
 *
 * returns <0 if failure, 0 on success
 */
static inline int
pci_read(struct platform_intf *intf, int bus, int dev, int func, int reg,
          size_t length, void *data)
{
	if (intf->op->pci->read(intf, bus, dev, func, reg, length, data)
	    != length) {
		return -1;
	}
	return 0;
}

/*
 * pci_read8  -  read 8 bits from PCI config space
 */
static inline int
pci_read8(struct platform_intf *intf, int bus, int dev, int func, int reg,
          uint8_t *data)
{
	return pci_read(intf, bus, dev, func, reg, sizeof(*data), data);
}

/*
 * pci_read16  -  read 16 bits from PCI config space
 */
static inline int
pci_read16(struct platform_intf *intf, int bus, int dev, int func, int reg,
           uint16_t *data)
{
	return pci_read(intf, bus, dev, func, reg, sizeof(*data), data);
}

/*
 * pci_read32  -  read 32 bits from PCI config space
 */
static inline int
pci_read32(struct platform_intf *intf, int bus, int dev, int func, int reg,
           uint32_t *data)
{
	return pci_read(intf, bus, dev, func, reg, sizeof(*data), data);
}

/*
 * pci_read64  -  read 64 bits from PCI config space
 */
static inline int
pci_read64(struct platform_intf *intf, int bus, int dev, int func, int reg,
           uint64_t *data)
{
	return pci_read(intf, bus, dev, func, reg, sizeof(*data), data);
}

/*
 * pci_write  -  write to PCI config space
 *
 * @intf:     platform interface
 * @bus:      PCI bus
 * @dev:      PCI device
 * @func:     PCI function
 * @reg:      configuration register
 * @length:   number of bytes to write
 * @data:     buffer to write data into
 *
 * returns <0 if failure, 0 on success
 */
static inline int
pci_write(struct platform_intf *intf, int bus, int dev, int func, int reg,
          size_t length, const void *data)
{
	if (intf->op->pci->write(intf, bus, dev, func, reg, length, data)
	    != length) {
		return -1;
	}
	return 0;
}

/*
 * pci_write8  -  write 8 bits to PCI config space
 */
static inline int
pci_write8(struct platform_intf *intf, int bus, int dev, int func, int reg,
          uint8_t data)
{
	return pci_write(intf, bus, dev, func, reg, sizeof(data), &data);
}

/*
 * pci_write16  -  write 16 bits to PCI config space
 */
static inline int
pci_write16(struct platform_intf *intf, int bus, int dev, int func, int reg,
           uint16_t data)
{
	return pci_write(intf, bus, dev, func, reg, sizeof(data), &data);
}

/*
 * pci_write32  -  write 32 bits to PCI config space
 */
static inline int
pci_write32(struct platform_intf *intf, int bus, int dev, int func, int reg,
           uint32_t data)
{
	return pci_write(intf, bus, dev, func, reg, sizeof(data), &data);
}

/*
 * pci_write64  -  write 64 bits to PCI config space
 */
static inline int
pci_write64(struct platform_intf *intf, int bus, int dev, int func, int reg,
           uint64_t data)
{
	return pci_write(intf, bus, dev, func, reg, sizeof(data), &data);
}

/*
 * pci_foreach  -  Execute function on all devices
 *
 * @intf:	platform interface
 * @cb:		pci callback
 * @data:	data supplied to callback
 *
 * returns the number returned by callback function
 * returns 0 to indicate success, failure otherwise.
 */
static inline int
pci_foreach(struct platform_intf *intf, pci_callback_t cb, void *data)
{
	return intf->op->pci->foreach(intf, cb, data);
}

/*
 * pci_foreach_in_bus  -  Execute function on all devices on a bus
 *
 * @intf:	platform interface
 * @bus:	target pci bus
 * @cb:		pci callback
 * @data:	data supplied to callback
 *
 * returns the number returned by callback function.
 * returns 0 to indicate success, failure otherwise.
 */
static inline int
pci_foreach_in_bus(struct platform_intf *intf, int bus,
                   pci_callback_t cb, void *data)
{
	return intf->op->pci->foreach_in_bus(intf, bus, cb, data);
}

#endif /* MOSYS_INTF_PCI_H__ */
