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
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/big_lock.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/acpi.h"
#include "lib/file.h"
#include "lib/probe.h"
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
		struct smbios_table table;
		if (smbios_find_table(intf, SMBIOS_TYPE_SYSTEM, 0, &table,
				      SMBIOS_LEGACY_ENTRY_BASE,
				      SMBIOS_LEGACY_ENTRY_LEN) == 0)
			id = table.string[table.data.system.name];
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

static int _release_lock(void)
{
#if defined(CONFIG_USE_IPC_LOCK)
	return mosys_release_big_lock() >= 0;
#endif
	return 0;
}

static int _acquire_lock()
{
#if defined(CONFIG_USE_IPC_LOCK)
	/* Timeout copied from lib/flashrom/flashrom.c */
	if (mosys_acquire_big_lock(50000) < 0) {
		lprintf(LOG_DEBUG, "%s: could not re-acquire lock\n",
			__func__);
		return -1;
	}
#endif
	return 0;
}

/*
 * Strips the "end of line" character (\n) in string.
 */
static void _strip_eol(char *str)
{
	char *newline = strchr(str, '\n');
	if (newline)
		*newline = '\0';
}

/*
 * Reads one stripped line from fp and close file.
 *
 * This is a helper utility for functions reading identifier files.
 */
static char *_read_close_stripped_line(FILE *fp)
{
	char buffer[256];

	if (!fp)
		return NULL;

	if (!fgets(buffer, sizeof(buffer), fp)) {
		buffer[0] = '\0';
	} else {
		_strip_eol(buffer);
	}
	fclose(fp);

	if (!*buffer)
		return NULL;
	return mosys_strdup(buffer);
}

/*
 * Reads and returns a VPD value.
 */
static char *_get_vpd_value(const char *key_name)
{
	char command[PATH_MAX];
	FILE *fp = NULL;
	int relock = 0;
	char *value;

	snprintf(command, sizeof(command), "vpd_get_value %s", key_name);
	command[sizeof(command) - 1] = '\0';

	relock = _release_lock();
	fp = popen(command, "r");
	value = _read_close_stripped_line(fp);
	if (relock)
		_acquire_lock();
	return value;
}

/*
 * Extracts the SERIES part from VPD "customization_id".
 *
 * customization_id should be in LOEMID-SERIES format.
 * If - is not found, return nothing.
 */
static char *_extract_customization_id_series_part(void)
{
	char *customization_id;
	char *series = NULL, *dash;

	customization_id = _get_vpd_value("customization_id");
	if (!customization_id)
		return NULL;

	dash = strchr(customization_id, '-');
	if (dash) {
		series = mosys_strdup(dash + 1);
	}
	free(customization_id);

	return series;
}

char *probe_brand(struct platform_intf *intf)
{
	const char *legacy_path = "/opt/oem/etc/BRAND_CODE";
	FILE *fp = fopen(legacy_path, "r");

	if (fp)
		return _read_close_stripped_line(fp);

	return _get_vpd_value("rlz_brand_code");
}

char *probe_chassis(struct platform_intf *intf)
{
	/* Fallback to customization_id (go/cros-chassis-id). */
	return _extract_customization_id_series_part();
}

int probe_sku_number(struct platform_intf *intf)
{
	if (!intf || !intf->cb || !intf->cb->sys) {
		return -1;
	}

	if (intf->cb->sys->sku_number) {
		return intf->cb->sys->sku_number(intf);
	}

	return -1;
}

#define FDT_MODEL_NODE	"/proc/device-tree/model"
char *fdt_model(void)
{
	int fd;
	static char model[32];
	int len;

	fd = file_open(FDT_MODEL_NODE, FILE_READ);
	if (fd < 0) {
		lperror(LOG_DEBUG, "Unable to open %s", FDT_MODEL_NODE);
		return NULL;
	}

	memset(model, 0, sizeof(model));
	len = read(fd, &model, sizeof(model));
	if (len < 0) {
		lprintf(LOG_DEBUG, "%s: Could not read FDT\n", __func__);
		return NULL;
	}

	return model;
}

#define FDT_COMPATIBLE	"/proc/device-tree/compatible"
int probe_fdt_compatible(const char *id_list[], int num_ids, int allow_partial)
{
	int ret = -1, i, fd;
	char path[PATH_MAX];
	char compat[64];	/* arbitrarily chosen max size */
	char *p;

	lprintf(LOG_DEBUG, "Probing platform with FDT compatible node\n");

	snprintf(path, PATH_MAX, "%s/%s",
			mosys_get_root_prefix(), FDT_COMPATIBLE);
	fd = file_open(path, FILE_READ);
	if (fd < 0) {
		lprintf(LOG_DEBUG, "Cannot open %s\n", path);
		return -1;
	}

	/*
	 * Device tree "compatible" data consists of a list of comma-separated
	 * pairs with a NULL after each pair. For example, "foo,bar\0bam,baz\0"
	 * is foo,bar and bam,baz.
	 */
	p = &compat[0];
	while (read(fd, p, 1) == 1) {
		if (*p != 0) {
			p++;
			if (p - &compat[0] > sizeof(compat)) {
				lprintf(LOG_ERR,
					"FDT compatible identifier too long\n");
				goto probe_fdt_compatible_exit;
			}
			continue;
		}

		for (i = 0; (i < num_ids) && id_list[i]; i++) {
			int cmp = 0;

			lprintf(LOG_DEBUG, "\t\"%s\" == \"%s\" ? ",
					&compat[0], id_list[i]);

			if (allow_partial) {
				cmp = strncmp(&compat[0],
					id_list[i], strlen(id_list[i]));
			} else {
				cmp = strcmp(&compat[0], id_list[i]);
			}

			if (!cmp) {
				lprintf(LOG_DEBUG, "yes\n");
				ret = i;
				goto probe_fdt_compatible_exit;
			} else {
				lprintf(LOG_DEBUG, "no\n");
			}
		}

		p = &compat[0];
	}

probe_fdt_compatible_exit:
	close(fd);
	return ret;
}

struct cros_compat_tuple *cros_fdt_tuple(void)
{
	struct cros_compat_tuple *ret = NULL;
	int fd;
	char path[PATH_MAX];
	char compat[64];	/* arbitrarily chosen max size */
	char family[32];
	char name[32];
	char revision[8];
	char *endptr;		/* end of current compat string */
	char *p0, *p1;		/* placeholders for detokenizing string */

	snprintf(path, PATH_MAX, "%s/%s",
			mosys_get_root_prefix(), FDT_COMPATIBLE);
	fd = file_open(path, FILE_READ);
	if (fd < 0) {
		lprintf(LOG_DEBUG, "Cannot open %s\n", path);
		return NULL;
	}

	/*
	 * Device tree "compatible" data consists of a list of comma-separated
	 * pairs with a NULL after each pair. For example, "foo,bar\0bam,baz\0"
	 * is foo,bar and bam,baz.
	 */
	endptr = &compat[0];
	while (read(fd, endptr, 1) == 1) {
		if (*endptr != 0) {
			endptr++;
			if (endptr - &compat[0] > sizeof(compat)) {
				lprintf(LOG_ERR,
					"FDT compatible identifier too long\n");
				break;
			}
			continue;
		}

		p0 = &compat[0];
		if (strncmp(p0, "google,", strlen("google,"))) {
			endptr = &compat[0];
			continue;
		}
		p0 += strlen("google,");

		/* family */
		p1 = strchr(p0, '-');
		if (p1 == NULL) {
			endptr = &compat[0];
			continue;
		}
		snprintf(family, p1 - p0 + 1, "%s", p0);

		/* name */
		p0 = p1 + 1;
		p1 = strchr(p0, '-');
		if (p1 == NULL) {
			endptr = &compat[0];
			continue;
		}
		snprintf(name, p1 - p0 + 1, "%s", p0);

		/* revision */
		p0 = p1 + 1;
		if (sscanf(p0, "%s", revision) != 1) {
			endptr = &compat[0];
			continue;
		}

		lprintf(LOG_DEBUG, "%s: family: %s, name: %s, revision: %s\n",
				__func__, family, name, revision);

		ret = mosys_malloc(sizeof(*ret));
		ret->family = mosys_strdup(family);
		ret->name = mosys_strdup(name);
		ret->revision = mosys_strdup(revision);
		add_destroy_callback(free, (void *)ret->family);
		add_destroy_callback(free, (void *)ret->name);
		add_destroy_callback(free, (void *)ret->revision);
		add_destroy_callback(free, (void *)ret);

		break;
	}

	close(fd);
	return ret;
}
