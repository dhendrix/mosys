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

#ifndef MOSYS_DRIVERS_EC_NUVOTON_NPCE781_H__
#define MOSYS_DRIVERS_EC_NUVOTON_NPCE781_H__

/* SuperI/O related definitions and functions. */
/* Strapping options */
#define NUVOTON_SIO_PORT1	0x2e	/* No pull-down resistor */
#define NUVOTON_SIO_PORT2	0x164e	/* Pull-down resistor on BADDR0 */
/* Note: There's another funky state that we won't worry about right now */

/* SuperI/O Config */
#define NPCE781_SIOCFG_SRID	0x27	/* SuperI/O Revision ID */
#define NPCE781_LDN_SHM		0x0f    /* LDN of SHM module */
#define NPCE781_LDN_PM2		0x12    /* LDN of SHM module */

/* NPCE781 shared memory config registers (LDN 0x0f) */
#define NPCE781_SHM_BASE_MSB		0x60
#define NPCE781_SHM_BASE_LSB		0x61
#define NPCE781_SHM_CFG			0xf0
#define NPCE781_SHM_CFG_BIOS_FWH_EN	(1 << 3)
#define NPCE781_SHM_CFG_FLASH_ACC_EN	(1 << 2)
#define NPCE781_SHM_CFG_BIOS_EXT_EN	(1 << 1)
#define NPCE781_SHM_CFG_BIOS_LPC_EN	(1 << 0)
#define NPCE781_WIN_CFG			0xf1	/* window config */
#define NPCE781_WIN_CFG_SHWIN_ACC	(1 << 6)

/* Shared access window 2 bar address registers */
#define NPCE781_SHAW2BA_0		0xf8
#define NPCE781_SHAW2BA_1		0xf9
#define NPCE781_SHAW2BA_2		0xfa
#define NPCE781_SHAW2BA_3		0xfb

/* Read/write buffer size */
#define NPCE781_MAX_WRITE_SIZE		 8
#define NPCE781_MAX_READ_SIZE		12

/* Mailbox interface */
#define NPCE781_MBX_CSR_OBF		(1 << 0)
#define NPCE781_MBX_CSR_IBF		(1 << 1)

struct platform_intf;	/* forward declare */

/*
 * npce781_get_sioport - return port used for super i/o config
 *
 * @intf:	platform interface
 * @port:	buffer to fill
 *
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
extern int npce781_get_sioport(struct platform_intf *intf, uint16_t *port);

/*
 * it8500_detect - detect ITE 8500 EC
 *
 * @intf:	platform interface
 *
 * returns 1 to indicate it8500 found
 * returns 0 if no EC found, but no error occurred
 * returns <0 to indicate error
 */
extern int npce781_detect(struct platform_intf *intf);

/*
 * npce781_read_csr  - read config/status register
 *
 * @intf:	platform interface
 * @port:	Super IO port to access
 * @ldn:	logical device number
 * @reg:	register to read from LDN
 *
 * returns 0 if no EC found, but no error occurred
 * returns <0 to indicate error
 */
extern uint8_t npce781_read_csr(struct platform_intf *intf, uint16_t port,
                                uint8_t ldn, uint8_t reg);

#endif	/* MOSYS_DRIVERS_EC_NUVOTON_NPCE781_H__ */
