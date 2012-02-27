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
 * cpuid.h: CPUID instruction interface
 */

#ifndef MOSYS_INTF_CPUID_H_
#define MOSYS_INTF_CPUID_H_

#include <inttypes.h>
#include "platform.h"

struct cpuid_data {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
};

struct cpuid_intf {
	/* this is exposed for testing, users should never use it */
	int (*do_cpuid_cur_cpu)(uint32_t op, uint32_t count,
	                        struct cpuid_data *data);

	/*
	 * cpuid - Generic CPUID interface
	 *
	 * @intf:	platform interface
	 * @result:	the cpuid result struct to fill out
	 * @core:	the core to execute CPUID instruction on
	 * @op:		the level to get information from
	 * @count:	the count (or level) input for ECX
	 *
	 * returns 0 to indicate success
	 * returns <0 to indicate failure
	 */
	int (*cpuid)(struct platform_intf *intf,
	             int core, uint32_t op, uint32_t count,
	             struct cpuid_data *result);

	/*
	 * setup  -  prepare interface
	 *
	 * @intf:	platform interface
	 *
	 * returns 0 to indicate success
	 * returns <0 to indicate failure
	 */
	int (*setup)(struct platform_intf *intf);

	/*
	 * destroy  -  teardown interface
	 *
	 * @intf:	platform interface
	 */
	void (*destroy)(struct platform_intf *intf);
};

/* cpu information based on cpuid instruction */
extern struct cpuid_intf cpuid_sys_intf;

/*
 * The following are helpers to make call-sites easier
 */

static inline int
cpuid(struct platform_intf *intf, int core, uint32_t op, uint32_t count,
      struct cpuid_data *result)
{
	return intf->op->cpuid->cpuid(intf, core, op, count, result);
}

#endif /* MOSYS_INTF_CPUID_H_ */
