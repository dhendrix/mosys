/* Copyright 2010, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *    * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <string.h>

#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/acpi.h"
#include "lib/smbios.h"
#include "lib/string.h"

int probe_hwid(const char *hwids[])
{
	char *id;
	int ret = 0;

	if (acpi_get_hwid(&id) < 0)
		return 0;

	if (strlfind(id, hwids, 0)) {
		ret = 1;
		lprintf(LOG_DEBUG, "%s: matched id \"%s\"\n", __func__, id);
	}
	free(id);
	return ret;
}

int probe_smbios(struct platform_intf *intf, const char *ids[])
{
	char *id;
	int ret = 0;

	/* Attempt to obtain platform ID string using sysinfo callback */
	if (intf && intf->cb && intf->cb->sysinfo && intf->cb->sysinfo->name)
		id = intf->cb->sysinfo->name(intf);

	/* Attempt to obtain platform ID string using raw SMBIOS */
	if (!id) {
		id = smbios_find_string(intf,
		                       SMBIOS_TYPE_SYSTEM,
		                       1,
		                       SMBIOS_LEGACY_ENTRY_BASE,
		                       SMBIOS_LEGACY_ENTRY_LEN);
	}

	if (!id) {
		ret = 0;
	} else if (strlfind(id, ids, 0)) {
		ret = 1;
		lprintf(LOG_DEBUG, "%s: matched id \"%s\"\n", __func__, id);
	}
	return ret;
}
