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

#ifndef MOSYS_DRIVERS_EC_ENE_KB932_H__
#define MOSYS_DRIVERS_EC_ENE_KB932_H__

#include <inttypes.h>

/* command interface */
#define ACPI_CSR		0x66	/* ACPI command/status port */
#define ACPI_DATA		0x62	/* ACPI data port */
#define KB932_SIDEBAND_CSR	0x6c	/* sideband command/status port */
#define KB932_SIDEBAND_DATA	0x68	/* sideband data port */

#define KB932_CMD_READ_ECRAM	0x80
#define KB932_CMD_WRITE_ECRAM	0x81

enum ene_ec {
	ENE_UNKNOWN	= 0,
	ENE_KB932	= 0xa2,
	ENE_KB3940	= 0xa3,
};

struct kb932_priv {
	/*
	 * Command/status and data port pairs. This should be defined
	 * on a per-platform basis. Examples: Legacy (0x64/0x60),
	 * ACPI (0x66/0x62), sideband (0x6c, 0x68), etc.
	 */
	uint8_t csr;		/* command/status port */
	uint8_t data;		/* data port */

	/* programmable address for banked EC module register access */
	uint32_t reg_base;

	unsigned int cmd_timeout_ms;	/* max command timeout period (ms) */
};

/**
 * Wait for EC firmware input buffer empty
 *
 * @param intf          platform_intf
 * @return 0            success
 * @return -1           timeout
 */
int kb932_wait_ibf_clear(struct platform_intf *intf);

/**
 * Wait for EC firmware output buffer full
 *
 * @param intf          platform_intf
 * @return 0           success
 * @return -1          timeout
 */
int kb932_wait_obf_set(struct platform_intf *intf);

/**
 * Read ene internal sram
 *
 * @param address       16bit sram address
 * @return              8bit sram data
 */
uint8_t ene_read(struct platform_intf *intf, uint16_t port,
			uint16_t address);

/**
 * Write ene internal sram
 *
 * @param address       16bit sram address
 * @param data          8bit data
 */
void ene_write(struct platform_intf *intf, uint16_t port,
			uint16_t address, uint8_t data);

/**
 * Detect kb932 on specific port
 *
 * returns 1 to indicate success
 * returns 0 if no ene kb932 determined, but no error occurred
 */
enum ene_ec ene_kb932_detect(struct platform_intf *intf, uint16_t port);

/**
 * Determine ENE EC name
 *
 * returns name if known, otherwise "Unknown"
 */
const char *ene_name(enum ene_ec ec);

#endif /* MOSYS_DRIVERS_EC_ENE_KB932_H__ */
