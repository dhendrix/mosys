/*
 * Copyright 2011, Google Inc.
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

/* strings used in sysfs */
#define CYAPA_SYSFS_FIRMWARE_VERSION	"firmware_version"
#define CYAPA_SYSFS_HARDWARE_VERSION	"hardware_version"
#define CYAPA_SYSFS_PRODUCT_ID		"product_id"

/* register offsets */
#define CYAPA_REG_PRODUCT_ID		0x2a	/* ASCII */
#define CYAPA_REG_PRODUCT_ID_LEN	(0x34 - 0x2a + 1)
#define CYAPA_REG_FIRMWARE_MAJOR	0x39
#define CYAPA_REG_FIRMWARE_MINOR	0x3a
#define CYAPA_REG_HARDWARE_MAJOR	0x3b
#define CYAPA_REG_HARDWARE_MINOR	0x3c

/*
 * cypress_get_firmware_version - get firmware version
 *
 * @intf:       platform interface
 * @bus:	i2c bus number
 * @addr:	i2c device address
 * @buf:	pointer to location to store result
 *
 * This function will allocate memory and store the firmware version as a
 * NULL-terminated string in the locatation pointed to by buf.
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
int cyapa_get_firmware_version(struct platform_intf *intf,
                               uint8_t bus, uint8_t addr, char **buf);

/*
 * cypress_get_hardware_version - get hardware version
 *
 * @intf:       platform interface
 * @bus:	i2c bus number
 * @addr:	i2c device address
 * @buf:	pointer to location to store result
 *
 * This function will allocate memory and store the hardware version as a
 * NULL-terminated string in the locatation pointed to by buf.
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int cyapa_get_hardware_version(struct platform_intf *intf,
                                      uint8_t bus, uint8_t addr, char **buf);

/*
 * cypress_get_product_id - get product ID
 *
 * @intf:       platform interface
 * @bus:	i2c bus number
 * @addr:	i2c device address
 * @buf:	pointer to location to store result
 *
 * This function will allocate memory and store the product ID as a
 * NULL-terminated string in the locatation pointed to by buf.
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int cyapa_get_product_id(struct platform_intf *intf,
                                uint8_t bus, uint8_t addr, char **buf);
