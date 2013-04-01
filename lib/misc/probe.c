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
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1	/* for strcasestr() */
#endif

#include <ctype.h>
#include <string.h>
#include <limits.h>

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/acpi.h"
#include "lib/smbios.h"
#include "lib/string.h"

#ifndef LINE_MAX
#define LINE_MAX	512
#endif

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

int probe_frid(const char *hwids[])
{
	int ret = 0;
	off_t len;
	static char *id = NULL;

	if (!id) {
		char *raw_frid = NULL, *tmp;

		if (acpi_get_frid(&raw_frid) < 0)
			goto probe_frid_done;

		/* FRID begins with platform name, followed by a dot, followed
		 * by revision ID */
		tmp = strchr(raw_frid, '.');
		if (!tmp) {
			lprintf(LOG_DEBUG, "%s: Invalid FRID: \"%s\"\n",
			                   __func__, raw_frid);
			free(raw_frid);
			goto probe_frid_done;
		}

		len = tmp - raw_frid + 1;
		id = mosys_malloc(len + 1);
		snprintf(id, len, "%s", raw_frid);
		lprintf(LOG_DEBUG, "%s: Platform name: \"%s\"\n", __func__, id);
		free(raw_frid);
		add_destroy_callback(free, id);
	}

	if (strlfind(id, hwids, 0)) {
		lprintf(LOG_DEBUG, "%s: matched id \"%s\"\n", __func__, id);
		ret = 1;
	}

probe_frid_done:
	return ret;
}

int probe_smbios(struct platform_intf *intf, const char *ids[])
{
	static char *id = NULL;
	int ret = 0;

	if (id)
		goto probe_smbios_cmp;

	/* Attempt platform-specific SMBIOS handler if one exists, else use the
	 * default approach. */
	if (intf->cb->smbios && intf->cb->smbios->system_name) {
		id = intf->cb->smbios->system_name(intf);
	} else {
		id = smbios_find_string(intf,
		                       SMBIOS_TYPE_SYSTEM,
		                       1,
		                       SMBIOS_LEGACY_ENTRY_BASE,
		                       SMBIOS_LEGACY_ENTRY_LEN);
	}

probe_smbios_cmp:
	if (!id) {
		ret = 0;
		lprintf(LOG_SPEW, "%s: cannot find product name\n", __func__);
	} else if (strlfind(id, ids, 0)) {
		ret = 1;
		lprintf(LOG_DEBUG, "%s: matched id \"%s\"\n", __func__, id);
	}
	return ret;
}

int probe_cpuinfo(struct platform_intf *intf,
                  const char *key, const char *value)
{
	FILE *cpuinfo;
	int ret = 0;
	char path[PATH_MAX];
	int key_found = 0;
	char line[LINE_MAX], *ptr;

	sprintf(path, "%s/proc/cpuinfo", mosys_get_root_prefix());
	cpuinfo = fopen(path, "rb");
	if (!cpuinfo)
		return 0;

	while (!feof(cpuinfo)) {
		if (fgets(line, sizeof(line), cpuinfo) == NULL)
			break;
		ptr = line;

		if (!strncmp(ptr, key, strlen(key))) {
			key_found = 1;
			break;
		}
	}

	if (key_found) {
		int i;
		char tmp[LINE_MAX];

		ptr += strlen(key);
		while (isspace((unsigned char)*ptr) || (*ptr == ':'))
			ptr++;

		memset(tmp, 0, sizeof(tmp));
		for (i = 0; !isspace((unsigned char)*ptr); i++) {
			tmp[i] = *ptr;
			ptr++;
		}

		lprintf(LOG_DEBUG, "\"%s\" == \"%s\" ? ", tmp, value);
		if (strncasecmp(tmp, value, strlen(value))) {
			lprintf(LOG_DEBUG, "no\n");
		} else {
			lprintf(LOG_DEBUG, "yes\n");
			ret = 1;
		}
	}

	fclose(cpuinfo);
	return ret;
}

const char *extract_cpuinfo(const char *key)
{
	FILE *cpuinfo;
	char *ret = NULL;
	char path[PATH_MAX];

	sprintf(path, "%s/proc/cpuinfo", mosys_get_root_prefix());
	cpuinfo = fopen(path, "rb");
	if (!cpuinfo)
		return 0;

	while (!feof(cpuinfo)) {
		char line[LINE_MAX], *ptr;
		int i = 0;

		if (fgets(line, sizeof(line), cpuinfo) == NULL)
			break;
		ptr = line;

		if (strncmp(ptr, key, strlen(key)))
			continue;

		ptr += strlen(key);
		while (isspace((unsigned char)*ptr) || (*ptr == ':'))
			ptr++;

		ret = mosys_malloc(strlen(ptr) + 1);
		while (!isspace((unsigned char)*ptr)) {
			ret[i] = *ptr;
			ptr++;
			i++;
		}
		ret[i] = '\0';
	}

	fclose(cpuinfo);
	return (const char *)ret;
}

int probe_cmdline(const char *key, int cs)
{
	FILE *cmdline;
	char path[PATH_MAX];
	char line[LINE_MAX];
	int ret = 0;

	if ((cs < 0) || (cs > 1))
		return -1;

	sprintf(path, "%s/proc/cmdline", mosys_get_root_prefix());
	cmdline = fopen(path, "rb");
	if (!cmdline)
		goto probe_cmdline_done;

	if (fgets(line, sizeof(line), cmdline) == NULL)
		goto probe_cmdline_done;

	if (cs) {
		if (strstr(line, key))
			ret = 1;
	} else {
		if (strcasestr(line, key))
			ret = 1;
	}

	if (ret)
		lprintf(LOG_DEBUG, "Found match on kernel command-line\n", key);
probe_cmdline_done:
	fclose(cmdline);
	return ret;
}

/*
 * extract_block_device_model_name - Gets model name of block storage device.
 *
 * @device:	String containing device name, ex "sda".
 *
 * Returns pointer to allocated model name string on success, NULL on failure.
 */
const char *extract_block_device_model_name(const char *device)
{
	FILE *file;
	char *model_name = NULL;
	char path[PATH_MAX];
	int len;

	sprintf(path, "/sys/class/block/%s/device/model", device);
	file = fopen(path, "r");
	if (!file)
		return NULL;

	model_name = mosys_malloc(PATH_MAX);
	fgets(model_name, PATH_MAX, file);
	fclose(file);

	/* Remove trailing newline. */
	len = strlen(model_name);
	if (len > 0 && model_name[len-1] == '\n')
		model_name[len-1] = '\0';

	return (const char *)model_name;
}
