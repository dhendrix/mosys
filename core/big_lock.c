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
#include "mosys/big_lock.h"
#include "mosys/locks.h"

#include "ipc_lock.h"

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
