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

#ifndef MOSYS_DRIVERS_EC_SMSC_MEC1308_H__
#define MOSYS_DRIVERS_EC_SMSC_MEC1308_H__

/* For shared mailbox interface */
#define MEC1308_LDN_MBX				0x09	/* Mailbox interface */
#define MEC1308_MBX_REG_DATA_START		0x84
#define MEC1308_MBX_REG_DATA_END		0x91
#define MEC1308_MBX_DATA_LEN			(MEC1308_MBX_REG_DATA_END - \
                                                 MEC1308_MBX_REG_DATA_START)

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
