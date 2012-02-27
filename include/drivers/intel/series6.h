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

#ifndef MOSYS_DRIVERS_INTEL_SERIES6_H__
#define MOSYS_DRIVERS_INTEL_SERIES6_H__

struct platform_intf;
struct gpio_map;

/*
 * The 6-Series has one SMBus interface, so for the purpose of matching with
 * sysfs entries we don't need to worry about the exact IO address which appears
 * at the end of the string (e.g. "SMBus I801 adapter at 2000").
 */
#define SERIES6_SMBUS_ADAPTER	"SMBus I801 adapter"

/*
 * series6_get_bbs - get bios boot straps (bbs) value
 *
 * @intf:	platform interface
 *
 * returns BBS value to indicate success
 * returns <0 to indicate failure
 */
enum ich_snb_bbs series6_get_bbs(struct platform_intf *intf);

/*
  * series6_set_bbs - set bios boot straps (bbs) value
  *
  * @intf:	platform interface
  * @bbs:	bbs value
  *
  * returns 0 to indicate success
  * returns <0 to indicate failure
  */
int series6_set_bbs(struct platform_intf *intf, enum ich_snb_bbs bbs);

/*
 * series6_read_gpio  - read GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 *
 * returns GPIO state as 0 or 1
 * returns <0 on read failure
 */

int series6_read_gpio(struct platform_intf *intf, struct gpio_map *gpio);

/*
 * series6_set_gpio  - set GPIO status
 *
 * @intf:	platform interface
 * @gpio:	gpio map
 * @status:	0/1
 *
 * returns 0 if successful
 * returns <0 on read failure
 */
int series6_set_gpio(struct platform_intf *intf, struct gpio_map *gpio, int state);

/*
 * series6_gpio_list  -  list all GPIOs and their states
 *
 * @intf:	platform interface
 *
 * returns 0 if successful
 * returns <0 if failure
 */
extern int series6_gpio_list(struct platform_intf *intf);

/*
 * series6_global_reset - force global reset
 *
 * @intf:	platform interface
 *
 * Obviously this function should force the platform to die before
 * returning, but we have a return code anyway...
 *
 * returns 0 if successful
 * returns <0 if failure
 */
extern int series6_global_reset(struct platform_intf *intf);

#endif /* MOSYS_DRIVERS_INTEL_SERIES6_H__ */
