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

#define MEC1308_DEFAULT_SIO_PORT		0x2e
#define MEC1308_LDN_MBX				0x09	/* Mailbox interface */
#define MEC1308_DEFAULT_MBX_IOBAD		0xa00

/* For shared mailbox interface spec */

#define MEC1308_MBX_REG_CMD			0x82
#define MEC1308_MBX_REG_EXTCMD			0x83
#define MEC1308_MBX_REG_DATA_START		0x84
#define MEC1308_MBX_REG_DATA_END		0x91
#define MEC1308_MBX_DATA_LEN			(MEC1308_MBX_REG_DATA_END - \
                                                 MEC1308_MBX_REG_DATA_START)

#define MEC1308_MBX_CMD_FW_VERSION		0x83
#define MEC1308_MBX_CMD_FAN_RPM			0xBB

#define MEC1308_MAX_TIMEOUT_US			2000000	/* arbitrarily picked */
#define MEC1308_DELAY_US			5000

#define MEC1308_MBX_CMD_PASSTHRU		0x55	/* start command */
#define MEC1308_MBX_CMD_PASSTHRU_SUCCESS	0xaa	/* success code */
#define MEC1308_MBX_CMD_PASSTHRU_FAIL		0xfe	/* failure code */
#define MEC1308_MBX_CMD_PASSTHRU_ENTER		"PathThruMode"	/* not a typo */
#define MEC1308_MBX_CMD_PASSTHRU_START		"Start"
#define MEC1308_MBX_CMD_PASSTHRU_EXIT		"End_Mode"

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

/*
 * mec1308_sio_name - return EC name string via SuperIO chip ID lookup
 *
 * @intf:	platform interface
 *
 * Note: this function uses the common "mec1308" name, but it can also work
 * with other chips in this family, e.g. mec1310.
 *
 * returns pointer to name string if successful
 * returns NULL to indicate error
 */
extern const char *mec1308_sio_name(struct platform_intf *intf);

/*
 * mec1308_sio_vendor - return EC vendor string via SuperIO vendor ID lookup
 *
 * @intf:	platform interface
 *
 * Note: this function uses the common "mec1308" name, but it can also work
 * with other chips in this family, e.g. mec1310.
 *
 * returns pointer to vendor string if successful
 * returns NULL to indicate error
 */
extern const char *mec1308_sio_vendor(struct platform_intf *intf);

/*
 * mec1308_mbx_fw_version - obtain EC firmware version via mailbox
 *
 * @intf:	platform interface
 * @buf:	buffer to store version string in
 * @len:	bytes in version string
 *
 * This function issues a command to the EC via the mailbox interface
 * and copies the version info from the mailbox data registers into buf.
 *
 * returns 0 if successful
 * returns <0 to indicate error
 */
extern int mec1308_mbx_fw_version(struct platform_intf *intf,
                                  uint8_t *buf, int len);

/*
 * mec1308_mbx_setup - setup internal variables (e.g. mailbox address)
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 to indicate error
 */
extern int mec1308_mbx_setup(struct platform_intf *intf);

/*
 * mec1308_mbx_teardown - release any resources used
 *
 * @intf:	platform interface
 */
extern void mec1308_mbx_teardown(struct platform_intf *intf);

/*
 * mec1308_mbx_exit_passthru_mode - exit passthru mode
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 to indicate failure
 */
extern int mec1308_mbx_exit_passthru_mode(struct platform_intf *intf);

#endif	/* MOSYS_DRIVERS_EC_SMSC_MEC1308_H__ */
