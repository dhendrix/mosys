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
 */

#ifndef MOSYS_LIB_FLASHROM_H__
#define MOSYS_LIB_FLASHROM_H__

enum programmer_target {
	INTERNAL_BUS_I2C,
	INTERNAL_BUS_LPC,
	INTERNAL_BUS_SPI,
	HOST_FIRMWARE,
	EC_FIRMWARE,
};

/*
 * flashrom_read - Read ROM using Flashrom utility
 *
 * @buf:	output buffer
 * @size:	(expected) size of ROM
 * @target:	target ROM
 * @region:	region to include with -i (NULL to read entire ROM)
 *
 * This function reads the target ROM by calling Flashrom with appropriate
 * parameters. It will save the ROM to a temporary file and then copy the
 * file into the provided buffer.
 */
extern int flashrom_read(uint8_t *buf, size_t size,
                         enum programmer_target target, const char *region);

/*
 * flashrom_read_by_name - Partial read using Flashrom utility
 *
 * @buf:	double-pointer of buffer to allocate and fill
 * @target:	target ROM
 * @region:	region to include with -i
 *
 * This function reads the target ROM by calling Flashrom with appropriate
 * parameters. It will allocate the appropriate number of bytes in buf.
 *
 * returns number of bytes read from region to indicate success
 * returns <0 to indicate failure
 */
extern int flashrom_read_by_name(uint8_t **buf,
                         enum programmer_target target, const char *region);

/*
 * flashrom_write_by_name - Partial write using Flashrom utility
 *
 * @size:	size of the data to write
 * @buf:	pointer to the buffer to write
 * @target:	target ROM
 * @region:	region to include with -i
 *
 * This function reads the target ROM by calling Flashrom with appropriate
 * parameters. It will allocate the appropriate number of bytes in buf.
 *
 * returns number of bytes read from region to indicate success
 * returns <0 to indicate failure
 */
extern int flashrom_write_by_name(size_t size, uint8_t *buf,
                         enum programmer_target target, const char *region);

/*
 * flashrom_read_host_firmware_region - Read firmware region within ROM
 *
 * @buf:	double-pointer of buffer to allocate and fill
 *
 * This assumes that the name of the firmware region corresponds to a defined
 * eeprom's "content" description.
 *
 * returns number of bytes read (ie region size) to indicate success
 * returns <0 to indicate failure
 */
extern int flashrom_read_host_firmware_region(struct platform_intf *intf,
							uint8_t **buf);

/*
 * flashrom_get_rom_size - Obtain the ROM size by calling flashrom
 *
 * @intf:	the platform interface
 * @target:	target programmer (host, ec, etc)
 *
 * returns ROM size in bytes to indicate success
 * returns <0 to indicate failure
 */
extern int flashrom_get_rom_size(struct platform_intf *intf,
			enum programmer_target target);

#endif /* MOSYS_LIB_FLASHROM_H__ */
