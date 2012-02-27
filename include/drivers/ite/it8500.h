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

#ifndef MOSYS_DRIVERS_EC_ITE_IT8500_H__
#define MOSYS_DRIVERS_EC_ITE_IT8500_H__

#define IT8500_LDN_SWUC		0x04	/* System Wake-Up Control */
#define IT8500_LDN_KBC_MOUSE	0x05	/* KBC/Mouse Interface */
#define IT8500_LDN_KBC_KEYBOARD	0x06	/* KBC/Keyboard interface */
#define IT8500_LDN_SMFI		0x0F	/* Shared Memory / Flash Interface */
#define IT8500_LDN_BRAM		0x10	/* BRAM */
#define IT8500_LDN_PMC1		0x11	/* Power Management I/F Channel 1 */
#define IT8500_LDN_PMC2		0x12	/* Power Management I/F Channel 2 */

enum {
	IT8500_IOBAD0,
	IT8500_IOBAD1,
};

struct platform_intf;	/* forward declare */

/*
 * it8500_get_sioport - return port used for super i/o config
 *
 * @intf:	platform interface
 * @port:	buffer to fill
 *
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
extern int it8500_get_sioport(struct platform_intf *intf, uint16_t *port);

/*
 * it8500_detect - detect ITE 8500 EC
 *
 * @intf:	platform interface
 *
 * returns 1 to indicate it8500 found
 * returns 0 if no EC found, but no error occurred
 * returns <0 to indicate error
 */
extern int it8500_detect(struct platform_intf *intf);

/*
 * it8500_get_iobad - get io base address
 *
 * @intf:	platform interface
 * @bank:	Which IO base address (0 or 1)
 * @ldn:	logical device number
 *
 * returns 1 to indicate it8500 found
 * returns 0 if no EC found, but no error occurred
 * returns <0 to indicate error
 */
extern uint16_t it8500_get_iobad(struct platform_intf *intf,
                                 int bank, uint8_t ldn);

#endif	/* MOSYS_DRIVERS_EC_ITE_IT8500_H__ */
