/*
 * Copyright 2013, Google Inc.
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
 */

#include "mosys/alloc.h"
#include "mosys/platform.h"
#include "nyan.h"

static const char *nyan_get_name(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->name);

	return (const char *)ret;
}

static const char *nyan_get_vendor(struct platform_intf *intf)
{
	char *ret = NULL;

	if (!strncmp(intf->name, "Big", strlen(intf->name)))
		ret = mosys_strdup("Google");
	else if (!strncmp(intf->name, "Blaze", strlen(intf->name)))
		ret = mosys_strdup("Google");
	else if (!strncmp(intf->name, "Kitty", strlen(intf->name)))
		ret = mosys_strdup("Google");

	return ret;
}

static const char *nyan_get_version(struct platform_intf *intf)
{
	char *ret = NULL;

	ret = mosys_strdup(intf->version_id);

	return (const char *)ret;
}

struct sys_cb nyan_sys_cb = {
	.name		= &nyan_get_name,
	.vendor		= &nyan_get_vendor,
	.version	= &nyan_get_version,
};
