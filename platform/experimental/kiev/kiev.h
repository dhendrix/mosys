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

#ifndef EXPERIMENTAL_KIEV_H__
#define EXPERIMENTAL_KIEV_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define KIEV_HOST_FIRMWARE_ROM_SIZE		(8192 * 1024)

/* platform callbacks */
extern struct eeprom_cb kiev_eeprom_cb;	/* eeprom.c */
//extern struct gpio_cb kiev_gpio_cb;		/* gpio.c */
extern struct memory_cb kiev_memory_cb;	/* memory.c */
extern struct nvram_cb kiev_nvram_cb;		/* nvram.c */
extern struct sensor_cb kiev_sensor_cb;	/* sensors.c */
extern struct sys_cb kiev_sys_cb;		/* sys.c */
extern struct vpd_cb kiev_vpd_cb;		/* vpd.c */

/* functions called by setup routines */
extern int kiev_superio_setup(struct platform_intf *intf);
extern void kiev_superio_destroy(struct platform_intf *intf);
extern int kiev_vpd_setup(struct platform_intf *intf);
extern int kiev_eeprom_setup(struct platform_intf *intf);

#endif /* EXPERIMENTAL_KIEV_H_ */
