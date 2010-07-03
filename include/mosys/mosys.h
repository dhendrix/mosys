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
 * mosys.h: macros and headers for mosys-specific common functions
 */

#ifndef MOSYS_MOSYS_H__
#define MOSYS_MOSYS_H__

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>       /* abort */

/*
 * MOSYS_CHECK() method to provide CHECK()-like behavior.
 *
 * This is intended to be equivalent to assert() except that it is evaluated,
 * even if NDEBUG is set.
 */
#define MOSYS_CHECK(exp_) \
do { \
  if (!(exp_)) { \
    fprintf(stderr, \
            "assertion failed in file %s, line %d", __FILE__, __LINE__); \
    abort(); \
  } \
} while (0);

/*
 * MOSYS_DCHECK() method to provide DCHECK()-like behavior.
 *
 * This is intended to be equivalent to an assert() call.
 */
#ifdef NDEBUG
#define MOSYS_DCHECK ((void)0)
#else
#define MOSYS_DCHECK MOSYS_CHECK
#endif

#endif /* MOSYS_MOSYS_H__ */
