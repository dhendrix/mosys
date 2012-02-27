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
 *
 * math.c: implementations of some numerical utilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>

#include "lib/math.h"

/*
 * ctz - count trailing zeros
 *
 * @u:	Bit vector to count trailing zeros in.
 *
 * Counts bit positions of lower significance than that of the least significant
 * bit set. Based off of an algorithm from:
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 * returns count of trailing zeros
 */
int ctz(unsigned long long int u)
{
	int num_zeros;
	float f;

	if (u == 0)	 /* The algorithm will return -127 on this condition */
		return 0;
	
	f = (float)(u & (~u + 1));
	num_zeros = (*(unsigned int *)&f >> 23) - 0x7f;
	
	return num_zeros;
}

/*
 * logbase2 - Return log base 2 of the absolute value of n (2^r = abs(n)) of an
 * integer by using a cast to float method (Requires IEEE-754). 
 *
 * Note: We could just use log2() but that would require messing with our 
 * compilation and linking options and hacking around the n = 0 case in other 
 * areas of the code. 
 *
 * @n:	The number to find the log base 2 of
 * 
 * returns log2(n) if successful
 */
int logbase2(int n)
{
	float f;
	int r;

	/* This algorithm fails (Returns negative infinity) if n = 0. We'll be
	 * using it mostly in the context of CPU numbers, so we'll take the
	 * liberty of returning 0 instead of aborting */
	if (n == 0)
		return 0;
	
	f = (float)n;
	memcpy(&r, &f, sizeof(n));
	
	/* Isolate exponent and un-bias the exponent (Subtract +128) */
	r = ((r & 0x7F800000) >> 23) - 0x80;

	return r + 1;
}

/*
 * rolling8_csum  -  Bytewise rolling summation "checksum" of a buffer
 *
 * @buf:	buffer to sum
 * @len:	length of buffer
 */
uint8_t rolling8_csum(uint8_t *buf, size_t len)
{
	size_t i;
	uint8_t sum = 0;

	for (i = 0; i < len; ++i)
		sum += buf[i];
	return sum;
}

/*
  * zero8_csum - Calculates 8-bit zero-sum checksum
  *
  * @buf:	input buffer
  * @len:	length of buffer
  * 
  * The summation of the bytes in the array and the csum will equal zero
  * for 8-bit data size.
  *
  * returns checksum to indicate success
  */
uint8_t zero8_csum(uint8_t *buf, size_t len)
{
	uint8_t *u = buf;
	uint8_t csum = 0;

	while (u < buf + len) {
		csum += *u;
		u++;
	}

	return (0x100 - csum);
}
