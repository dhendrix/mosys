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

#ifndef MOSYS_LIB_UTIL_COMMON_H__
#define MOSYS_LIB_UTIL_COMMON_H__

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>       /* abort */

/* CPU designations */
enum cpu_types {
	CPU_NOT_PRESENT 	= -1,
	CPU_UNKNOWN		= 0,
};

/* Misc */

#include "mosys/list.h"
extern struct ll_node *scanft(struct ll_node **list,
                              const char *root, const char *name,
                              const char *str, int symdepth);
#ifndef offsetof
#define offsetof(type_, member_) ((size_t)&((type_ *)0)->member_)
#endif  // offsetof

#ifndef container_of
#define container_of(ptr_, type_, member_) ({ \
  const typeof(((type_ *)0)->member_) *__mptr = (ptr_); \
  (type_ *)((char *)__mptr - offsetof(type_, member_));})
#endif  // container_of

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

#endif /* MOSYS_LIB_UTIL_COMMON_H__ */
