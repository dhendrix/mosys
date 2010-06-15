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
 *
 */

#ifndef MOSYS_IPC_LOCK_H__
#define MOSYS_IPC_LOCK_H__

#include <sys/ipc.h>

/* a mosys lock */
struct ipc_lock {
	key_t key;         /* provided by the developer */
	int sem;           /* mosys internal */
	int is_held;       /* mosys internal */
};

/* don't use C99 initializers here, so this can be used in C++ code */
#define IPC_LOCK_INIT(key) \
	{ \
		key,       /* name */ \
		-1,        /* sem */ \
		0,         /* is_held */ \
	}

/*
 * mosys_lock: acquire a mosys lock
 *
 * timeout <0 = no timeout (try forever)
 * timeout 0  = do not wait (return immediately)
 * timeout >0 = wait up to $timeout milliseconds (subject to kernel scheduling)
 *
 * return 0   = lock acquired
 * return >0  = lock was already held
 * return <0  = failed to acquire lock
 */
extern int mosys_lock(struct ipc_lock *lock, int timeout_msecs);

/*
 * mosys_unlock: release a mosys lock
 *
 * returns 0 if lock was released successfully
 * returns -1 if lock had not been held before the call
 */
extern int mosys_unlock(struct ipc_lock *lock);

/*
 * WARNING!!!
 *
 * The following is exposed only for testing.  These must not be used
 * outside of tests and the implementation of the above interfaces.
 */

extern int mosys_lock_is_held(struct ipc_lock *lock);

#endif /* MOSYS_IPC_LOCK_H__ */
