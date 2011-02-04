/*
 * Copyright (C) 2010 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Google Inc. or the names of contributors or
 * licensors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * GOOGLE INC AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Note: This file shares some code with the Flashrom project.
 */

#ifndef MOSYS_DRIVERS_EC_SMSC_MEC1308_H__
#define MOSYS_DRIVERS_EC_SMSC_MEC1308_H__

#define MEC1308_SIO_ENTRY_KEY	0x55
#define MEC1308_SIO_EXIT_KEY	0xaa

#define MEC1308_LDN_MBX		0x09	/* Mailbox interface */

struct platform_intf;	/* forward declare */

extern void mec1308_sio_enter(struct platform_intf *intf, uint16_t port);
extern void mec1308_sio_exit(struct platform_intf *intf, uint16_t port);

/*
 * mec1308_get_sioport - return port used for super i/o config
 *
 * @intf:	platform interface
 * @port:	buffer to fill
 *
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
extern int mec1308_get_sioport(struct platform_intf *intf, uint16_t *port);

/*
 * mec1308_detect - detect mec1308 EC
 *
 * @intf:	platform interface
 *
 * returns 1 to indicate mec1308 found
 * returns 0 if no EC found, but no error occurred
 * returns <0 to indicate error
 */
extern int mec1308_detect(struct platform_intf *intf);

/*
 * mec1308_get_iobad - get io base address
 *
 * @intf:	platform interface
 * @port:	super i/o port
 * @ldn:	logical device number
 *
 * returns 1 to indicate mec1308 found
 * returns 0 if no EC found, but no error occurred
 * returns <0 to indicate error
 */
extern uint16_t mec1308_get_iobad(struct platform_intf *intf,
                                  uint16_t port, uint8_t ldn);

#endif	/* MOSYS_DRIVERS_EC_SMSC_MEC1308_H__ */
