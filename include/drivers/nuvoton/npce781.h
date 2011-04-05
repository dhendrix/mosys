/*
 * Copyright (C) 2011 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
