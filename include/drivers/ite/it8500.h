/*
 * Copyright (C) 2010 Google Inc.
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
