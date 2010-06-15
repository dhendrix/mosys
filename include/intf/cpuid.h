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
