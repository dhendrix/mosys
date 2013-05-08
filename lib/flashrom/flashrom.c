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
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
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
 * flashrom.c: flashrom wrappers
 */

#include <ctype.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mosys/alloc.h"
#include "mosys/big_lock.h"
#include "mosys/log.h"

#include "lib/flashrom.h"
#include "lib/string_builder.h"

/* returns pointer to string containing flashrom path if successful,
   returns NULL otherwise */
static const char *flashrom_path(void)
{
	FILE *fp;
	static char path[PATH_MAX];
	int c, i = 0;

	if ((fp = popen("which flashrom 2>/dev/null", "r")) == NULL) {
		lprintf(LOG_DEBUG, "Cannot find flashrom\n");
		return NULL;
	}

	for (i = 0; i < PATH_MAX; i++) {
		c = fgetc(fp);

		if (c == EOF || c == '\n') {
			path[i] = '\0';
			break;
		}
		path[i] = c;
	}

	pclose(fp);
	/* no characters were read from stream, or buffer overrun */
	if ((c == EOF && i == 0) || (c != EOF && i == PATH_MAX))
		return NULL;
	return path;
}

static int do_flashrom(const char *cmd)
{
	int rc = 0;
#if defined(CONFIG_USE_IPC_LOCK)
	int re_acquire_lock = 1;
#endif

#if defined(CONFIG_USE_IPC_LOCK)
	if (mosys_release_big_lock() < 0)
		re_acquire_lock = 0;
#endif

	if (system(cmd) != EXIT_SUCCESS) {
		lprintf(LOG_DEBUG, "%s: Failed to run %s\n", __func__, cmd);
		rc = -1;
	}

#if defined(CONFIG_USE_IPC_LOCK)
        /* try to get lock */
        if (re_acquire_lock && (mosys_acquire_big_lock(50000) < 0)) {
		lprintf(LOG_DEBUG, "%s: could not re-acquire lock\n", __func__);
		rc = -1;
        }
#endif

	return rc;
}

/* TODO: add arbitrary range support */
int flashrom_read(uint8_t *buf, size_t size,
                  enum target_bus target, const char *region)
{
	int fd, rc = -1;
	struct string_builder *sb = new_string_builder();
	char *flashrom_cmd;
	char filename[] = "/tmp/flashrom_XXXXXX";
	struct stat s;
	const char *path;

	if ((path = flashrom_path()) == NULL)
		goto flashrom_read_exit_1;

	string_builder_sprintf(sb, path);

	switch(target) {
	case INTERNAL_BUS_SPI:
		string_builder_strcat(sb, " -p internal:bus=spi");
		break;
	case INTERNAL_BUS_I2C:
		string_builder_strcat(sb, " -p internal:bus=i2c");
		break;
	case INTERNAL_BUS_LPC:
		string_builder_strcat(sb, " -p internal:bus=lpc");
		break;
	default:
		lprintf(LOG_DEBUG, "Unsupported target: %d\n", target);
		goto flashrom_read_exit_1;
	}

	if (!mkstemp(filename)) {
		lperror(LOG_DEBUG, "Unable to make temporary file for flashrom");
		goto flashrom_read_exit_1;
	}

	if (region) {
		string_builder_strcat(sb, " -i ");
		string_builder_strcat(sb, region);
	}

	string_builder_strcat(sb, " -r ");
	string_builder_strcat(sb, filename);
	string_builder_strcat(sb, " >/dev/null 2>&1");

	flashrom_cmd = mosys_strdup(string_builder_get_string(sb));
	lprintf(LOG_DEBUG, "Calling \"%s\"\n", flashrom_cmd);
	if (do_flashrom(flashrom_cmd) < 0)
		goto flashrom_read_exit_2;

	fd = open(filename, O_RDONLY);
	if (fstat(fd, &s) < 0) {
		lprintf(LOG_DEBUG, "%s: Cannot stat %s\n", __func__, filename);
		goto flashrom_read_exit_2;
	}

	if (s.st_size != size) {
		lprintf(LOG_DEBUG, "%s: Size of image: %lu, expected %lu",
		                   __func__, s.st_size, size);
		goto flashrom_read_exit_2;
	}

	if (read(fd, buf, size) != size) {
		lperror(LOG_DEBUG, "%s: Unable to read image");
		goto flashrom_read_exit_2;
	}

	rc = 0;
flashrom_read_exit_2:
	free(flashrom_cmd);
flashrom_read_exit_1:
	unlink(filename);
	free_string_builder(sb);
	return rc;
}
