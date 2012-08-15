/* Copyright 2012, Google Inc.
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

#include "mosys/big_lock.h"
#include "mosys/ipc_lock.h"
#include "mosys/locks.h"

static struct ipc_lock mosys_big_lock = IPC_LOCK_INIT(MOSYS_LOCK_BIGLOCK);

/*
 * mosys_acquire_big_lock  -  acquire global lock
 *
 * returns 0 to indicate lock acquired
 * returns >0 to indicate lock was already held
 * returns <0 to indicate failed to acquire lock
 */
int mosys_acquire_big_lock(int timeout_secs)
{
	return mosys_lock(&mosys_big_lock, timeout_secs*1000);
}

/*
 * mosys_release_big_lock  -  release global lock
 *
 * returns 0 if lock was released successfully
 * returns -1 if lock had not been held before the call
 */
int mosys_release_big_lock(void)
{
	return mosys_unlock(&mosys_big_lock);
}

/*
 * mosys_big_lock_prepare_test - setup the big lock for testing.
 *
 * This should only be used doing *TESTING*. We need to use a different key
 * for testing so we don't conflict with other mosys binaries or users of
 * libmosys.
 */
void mosys_big_lock_prepare_test(void)
{
	mosys_big_lock.key = MOSYS_LOCK_TEST_BIGLOCK;
}
