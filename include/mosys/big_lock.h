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

#ifndef MOSYS_BIG_LOCK_H__
#define MOSYS_BIG_LOCK_H__

extern int mosys_acquire_big_lock(int timeout_secs);
/*
 * mosys_release_big_lock  -  release global lock
 *
 * returns 0 if lock was released successfully
 * returns -1 if lock had not been held before the call
 */
extern int mosys_release_big_lock(void);
extern void mosys_big_lock_prepare_test(void);

#endif /* MOSYS_BIG_LOCK_H__ */
