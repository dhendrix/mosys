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

#ifndef PLATFORM_PARROT_H__
#define PLATFORM_PARROT_H__

#include <inttypes.h>
#include "mosys/platform.h"

#define PARROT_HOST_FIRMWARE_ROM_SIZE		(8192 * 1024)

/* platform callbacks */
extern struct eeprom_cb parrot_eeprom_cb;	/* eeprom.c */
extern struct memory_cb parrot_memory_cb;	/* memory.c */
extern struct nvram_cb parrot_nvram_cb;		/* nvram.c */
extern struct sys_cb parrot_sys_cb;		/* sys.c */
extern struct vpd_cb parrot_vpd_cb;		/* vpd.c */
extern struct legacy_ec_cb parrot_ec_cb;	/* ec.c */

/* functions called by setup routines */
extern int parrot_vpd_setup(struct platform_intf *intf);
extern int parrot_eeprom_setup(struct platform_intf *intf);

#endif /* PLATFORM_PARROT_H_ */
