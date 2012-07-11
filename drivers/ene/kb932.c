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

#include <inttypes.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "drivers/superio.h"
#include "drivers/ene/kb932.h"

#define ENE_HWVER 0xa2
#define ENE_EDIID 0x02

static const int port_ene_bank   = 1;
static const int port_ene_offset = 2;
static const int port_ene_data   = 3;

static const uint16_t ene_hwver_addr = 0xff00;
static const uint16_t ene_ediid_addr = 0xff24;

/**
 * Read ene internal sram
 *
 * @param address       16bit sram address
 * @return              8bit sram data
 */
uint8_t ene_read(struct platform_intf *intf, uint16_t port,
			uint16_t address)
{
	uint8_t bank   = address >> 8;
	uint8_t offset = address & 0xff;
	uint8_t data = 0xff;

	io_write8(intf, port + port_ene_bank, bank);
	io_write8(intf, port + port_ene_offset, offset);
	io_read8(intf, port + port_ene_data, &data);
	return data;
}

/**
 * Write ene internal sram
 *
 * @param address       16bit sram address
 * @param data          8bit data
 */
void ene_write(struct platform_intf *intf, uint16_t port,
			uint16_t address, uint8_t data)
{
	uint8_t bank   = address >> 8;
	uint8_t offset = address & 0xff;

	io_write8(intf, port + port_ene_bank, bank);
	io_write8(intf, port + port_ene_offset, offset);
	io_write8(intf, port + port_ene_data, data);
}

/*
 * returns 1 to indicate success
 * returns 0 if no ene kb932 determined, but no error occurred
 */
int ene_kb932_detect(struct platform_intf *intf, uint16_t port)
{
	if (ene_read(intf, port, ene_hwver_addr) != ENE_HWVER)
		return 0;
	if (ene_read(intf, port, ene_ediid_addr) != ENE_EDIID)
		return 0;

	return 1;
}

