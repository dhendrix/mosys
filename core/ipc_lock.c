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

#include <time.h>

#include "mosys/common.h"
#include "mosys/log.h"

#include "csem.h"
#include "ipc_lock.h"

/*
 * this function is exposed (non-static) for testing
 */
int
mosys_lock_is_held(struct ipc_lock *lock)
{
	if (lock->is_held) {
		return 1;
	}
	return 0;
}

static int
lock_init(struct ipc_lock *lock)
{
	if (lock->sem < 0) {
		/* get or create the semaphore, init to 1 if needed */
		int sem = csem_get_or_create(lock->key, 1);
		if (sem < 0) {
			return -1;  // COV_NF_LINE
		}
		lock->sem = sem;
	}
	return 0;
}

static void
msecs_to_timespec(int msecs, struct timespec *tmspec)
{
	tmspec->tv_sec = msecs / 1000;
	tmspec->tv_nsec = (msecs % 1000) * 1000 * 1000;
}

/*
 * mosys_lock - acquire a lock
 *
 * timeout <0 = no timeout (try forever)
 * timeout 0  = do not wait (return immediately)
 * timeout >0 = wait up to $timeout milliseconds
 *
 * returns 0 to indicate lock acquired
 * returns >0 to indicate lock was already held
 * returns <0 to indicate failed to acquire lock
 */
int
mosys_lock(struct ipc_lock *lock, int timeout_msecs)
{
	int ret;
	struct timespec timeout;
	struct timespec *timeout_ptr;

	/* initialize the lock */
	if (lock_init(lock) < 0) {
		lperror(LOG_ERR, "failed to init lock 0x%08x",  // COV_NF_LINE
		        (uint32_t)lock->key);                   // COV_NF_LINE
		return -1;                                      // COV_NF_LINE
	}

	/* check if it is already held */
	if (mosys_lock_is_held(lock)) {
		return 1;
	}

	/* calculate the timeout */
	if (timeout_msecs >= 0) {
		timeout_ptr = &timeout;
		msecs_to_timespec(timeout_msecs, timeout_ptr);
	} else {
		timeout_ptr = NULL;
	}

	/* try to get the lock */
	ret = csem_down_timeout_undo(lock->sem, timeout_ptr);
	if (ret < 0) {
		lperror(LOG_ERR, "failed to acquire lock 0x%08x",
		        (uint32_t)lock->key);
		return -1;
	}

	/* success */
	lock->is_held = 1;
	return 0;
}

/*
 * mosys_unlock - release a lock
 *
 * returns 0 if lock was released successfully
 * returns -1 if lock had not been held before the call
 */
int
mosys_unlock(struct ipc_lock *lock)
{
	if (mosys_lock_is_held(lock)) {
		lock->is_held = 0;
		csem_up_undo(lock->sem);
		/* NOTE: do not destroy the semaphore, we want it to persist */
		return 0;
	}
        /* did not hold the lock */
        return -1;
}
