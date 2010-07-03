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
 * common.h: useful declarations and macros
 */

#ifndef MOSYS_LIB_COMMON_H__
#define MOSYS_LIB_COMMON_H__

/* CPU designations */
enum cpu_types {
	CPU_NOT_PRESENT 	= -1,
	CPU_UNKNOWN		= 0,
};

#ifndef offsetof
#define offsetof(type_, member_) ((size_t)&((type_ *)0)->member_)
#endif  // offsetof

#ifndef container_of
#define container_of(ptr_, type_, member_) ({ \
  const typeof(((type_ *)0)->member_) *__mptr = (ptr_); \
  (type_ *)((char *)__mptr - offsetof(type_, member_));})
#endif  // container_of

#endif /* MOSYS_LIB_COMMON_H__ */
