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
