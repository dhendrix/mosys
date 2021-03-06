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
 *
 * locks.h: locks are used to preserve atomicity of operations in mosys.
 * This is useful if multiple instances of mosys might be used at once,
 * such as if a monitoring daemon links to it and a userspace app is present.
 */

#ifndef MOSYS_LOCKS_H__
#define MOSYS_LOCKS_H__

/* this is the base key, since we have to pick something global */
#define MOSYS_IPC_LOCK_KEY	(0x67736c00 & 0xfffffc00) /* 22 bits "gsl" */

/* The ordering of the following keys matters a lot. We don't want to reorder
 * keys and have a new mosys or binary dependent on deployed/used
 * because it will break the atomicity of existing mosys users and binaries. In
 * other words, DO NOT REORDER. */

/* this is the mosys "big lock". */
#define MOSYS_LOCK_BIGLOCK	(MOSYS_IPC_LOCK_KEY + 0)

/* for Google EC */
#define CROS_EC_LOCK		(MOSYS_IPC_LOCK_KEY + 1)

/* key for testing, should never be used in production */
#define MOSYS_TEST_KEY		0xffffffff

/* For alternative file locking mechanism */
#define SYSTEM_LOCKFILE_DIR	"/run/lock"
#define MOSYS_LOCKFILE_NAME	"firmware_utility_lock"
#define CROS_EC_LOCKFILE_NAME	"cros_ec_lock"

#endif /* MOSYS_LOCKS_H__ */

