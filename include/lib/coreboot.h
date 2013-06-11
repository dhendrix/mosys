/*
 * Copyright 2013, Google Inc.
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

#ifndef MOSYS_LIB_COREBOOT_H_
#define MOSYS_LIB_COREBOOT_H_

#define POST_RESET_VECTOR_CORRECT		0x01
#define POST_ENTER_PROTECTED_MODE		0x10
#define POST_PREPARE_RAMSTAGE 			0x11
#define POST_ENTRY_C_START			0x13
#define POST_PRE_HARDWAREMAIN			0x79
#define POST_ENTRY_RAMSTAGE			0x80
#define POST_CONSOLE_READY			0x39
#define POST_CONSOLE_BOOT_MSG			0x40
#define POST_ENABLING_CACHE			0x60
#define POST_BS_PRE_DEVICE			0x70
#define POST_BS_DEV_INIT_CHIPS			0x71
#define POST_BS_DEV_ENUMERATE			0x72
#define POST_BS_DEV_RESOURCES			0x73
#define POST_BS_DEV_ENABLE			0x74
#define POST_BS_DEV_INIT			0x75
#define POST_BS_POST_DEVICE			0x76
#define POST_BS_OS_RESUME_CHECK			0x77
#define POST_BS_OS_RESUME			0x78
#define POST_BS_WRITE_TABLES			0x79
#define POST_BS_PAYLOAD_LOAD			0x7a
#define POST_BS_PAYLOAD_BOOT			0x7b
#define POST_ENTER_ELF_BOOT			0xf8
#define POST_JUMPING_TO_PAYLOAD			0xf3
#define POST_DEAD_CODE				0xee
#define POST_OS_RESUME				0xfd
#define POST_OS_BOOT				0xfe
#define POST_DIE 				0xff

#endif /* MOSYS_LIB_COREBOOT_H_ */
