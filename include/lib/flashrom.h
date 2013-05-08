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

enum target_bus {
	INTERNAL_BUS_I2C,
	INTERNAL_BUS_LPC,
	INTERNAL_BUS_SPI,
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
                         enum target_bus target, const char *region);

/*
 * flashrom_read_fmap_area - Partial read using Flashrom utility
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
                         enum target_bus target, const char *region);

#endif /* MOSYS_LIB_FLASHROM_H__ */
