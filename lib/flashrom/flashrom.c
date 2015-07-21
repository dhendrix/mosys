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
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "mosys/alloc.h"
#include "mosys/big_lock.h"
#include "mosys/log.h"

#include "lib/flashrom.h"

#define MAX_ARRAY_SIZE 256

static int in_android = 0;

/* returns pointer to string containing flashrom path if successful,
   returns NULL otherwise */
static const char *flashrom_path(void)
{
	static char path[PATH_MAX];
	FILE *fp;
	int fd;
	int c, i = 0;
	struct stat s;
	char android_path[] = "/system/bin/flashrom";

	/* In Android, flashrom utility located in /system/bin
	   check if file exists.  Using fstat because for some
	   reason, stat() was seg faulting in Android */
	fd = open(android_path, O_RDONLY);
	if (fstat(fd, &s) == 0) {
		in_android = 1;
		strcpy(path, android_path);
		close(fd);
		return path;
	}
	close(fd);

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

/*
  Wrapper function to execute flashrom

  Inputs:
    cmd:  path to flashrom binary
    argv: array containing arguments being passed to flashrom

  Return:
    -1 on error, 0 on success
 */
static int do_flashrom(const char *cmd, char *const *argv)
{
	int rc = 0;
#if defined(CONFIG_USE_IPC_LOCK)
	int re_acquire_lock = 1;
#endif
	int pid = -1;
	int status = 0;
	int fd, i;

#if defined(CONFIG_USE_IPC_LOCK)
	if (mosys_release_big_lock() < 0)
		re_acquire_lock = 0;
#endif
	if (argv != NULL) {
		for (i = 0; argv[i] != NULL; i++) {
			lprintf(LOG_DEBUG, "%s ", argv[i]);
		}
		lprintf(LOG_DEBUG, "\n");
	}

	if ((pid = fork()) < 0) {
		lprintf(LOG_ERR, "fork() error\n");
		rc = -1;
	} else if (pid == 0) { /* child */
		fd = open("/dev/null", O_WRONLY);
		dup2(fd, 1);
		dup2(fd, 2);

		execv(cmd, argv);

		lperror(LOG_ERR, "%s: Failed to run %s", __func__, cmd);
		rc = -1;
	} else { /* parent */
		if (waitpid(pid, &status, 0) < 0 || status) {
			lprintf(LOG_ERR, "waitpid returns error\n");
		}
		close(fd);
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

/*
   Populates prog_args with programmer args
   return -1 on error, otherwise returns # of args

   Inputs:
     first_empty_slot:  Next empty slot in array
     programmer_target: target

   Output:
     prog_arg:          output args

   Return:
     Number of entries in the array that were allocated
*/

static int append_programmer_arg(const enum programmer_target target,
				 const int first_empty_slot,
				 char **prog_arg)
{
	int ret = 0;
	int slot = first_empty_slot;

	switch(target) {
	case INTERNAL_BUS_SPI:
		prog_arg[slot++] = strdup("-p");
		prog_arg[slot++] = strdup("internal:bus=spi");
		ret = 2;
		break;
	case INTERNAL_BUS_I2C:
		prog_arg[slot++] = strdup("-p");
		prog_arg[slot++] = strdup("internal:bus=i2c");
		ret = 2;
		break;
	case INTERNAL_BUS_LPC:
		prog_arg[slot++] = strdup("-p");
		prog_arg[slot++] = strdup("internal:bus=lpc");
		ret = 2;
		break;
	case HOST_FIRMWARE:
		prog_arg[slot++] = strdup("-p");
		prog_arg[slot++] = strdup("host");
		ret = 2;
		break;
	case EC_FIRMWARE:
		prog_arg[slot++] = strdup("-p");
		prog_arg[slot++] = strdup("ec");
		ret = 2;
		break;
	default:
		lprintf(LOG_DEBUG, "Unsupported target: %d\n", target);
		ret = -1;
	}

	return ret;
}

/* TODO: add arbitrary range support */
int flashrom_read(uint8_t *buf, size_t size,
                  enum programmer_target target, const char *region)
{
	int fd, rc = -1;
	char filename[] = "flashrom_XXXXXX";
	char full_filename[PATH_MAX];
	char *args[MAX_ARRAY_SIZE];
	struct stat s;
	const char *path;
	int i = 0;

	if ((path = flashrom_path()) == NULL)
		goto flashrom_read_exit_0;
	args[i++] = strdup(path);

	if ((i += append_programmer_arg(target, i, args)) < 0)
		goto flashrom_read_exit_0;

	if (in_android == 1) {
		/* In Android, /data is writable */
		strcpy(full_filename, "/data/");
	} else {
		strcpy(full_filename, "/tmp/");
	}
	strcat(full_filename, filename);
	if (mkstemp(full_filename) == -1) {
		lperror(LOG_DEBUG,
			"Unable to make temporary file for flashrom");
		goto flashrom_read_exit_0;
	}

	if (region) {
		args[i++] = strdup("-i");
		args[i++] = strdup(region);
	}
	args[i++] = strdup("-r");
	args[i++] = strdup(full_filename);
	args[i++] = NULL;

	if (do_flashrom(path, args) < 0)
		goto flashrom_read_exit_1;

	fd = open(full_filename, O_RDONLY);
	if (fstat(fd, &s) < 0) {
		lprintf(LOG_DEBUG, "%s: Cannot stat %s\n", __func__, full_filename);
		goto flashrom_read_exit_1;
	}

	if (s.st_size != size) {
		lprintf(LOG_DEBUG, "%s: Size of image: %lu, expected %lu\n",
		                   __func__, s.st_size, size);
		goto flashrom_read_exit_1;
	}

	if (read(fd, buf, size) != size) {
		lperror(LOG_DEBUG, "%s: Unable to read image\n");
		goto flashrom_read_exit_1;
	}

	rc = 0;
flashrom_read_exit_1:
	for (i = 0; args[i] != NULL; i++)
		free(args[i]);
flashrom_read_exit_0:
	unlink(full_filename);
	return rc;
}

int flashrom_read_by_name(uint8_t **buf,
                  enum programmer_target target, const char *region)
{
	int fd, rc = -1;
	struct stat s;
	const char *path;
	char filename[] = "flashrom_XXXXXX";
	char full_filename[PATH_MAX];
	char *args[MAX_ARRAY_SIZE];
	char region_file[MAX_ARRAY_SIZE];
	int i = 0;

	if (!region)
		goto flashrom_read_exit_0;

	if ((path = flashrom_path()) == NULL)
		goto flashrom_read_exit_0;
	args[i++] = strdup(path);

	if ((i += append_programmer_arg(target, i, args)) < 0)
		goto flashrom_read_exit_1;

	if (in_android == 1) {
		/* In Android, no tmp, but /data is writable */
		strcpy(full_filename, "/data/");
	} else {
		strcpy(full_filename, "/tmp/");
	}
	strcat(full_filename, filename);
	if (mkstemp(full_filename) == -1) {
		lperror(LOG_DEBUG,
			"Unable to make temporary file for flashrom");
		goto flashrom_read_exit_1;
	}

	args[i++] = strdup("-i");
	strcpy(region_file, region);
	strcat(region_file, ":");
	strcat(region_file, full_filename);
	args[i++] = strdup(region_file);
	args[i++] = strdup("-r");
	args[i++] = NULL;

	if (do_flashrom(path, args) < 0) {
		lprintf(LOG_DEBUG, "Unable to read region \"%s\"\n", region);
		goto flashrom_read_exit_2;
	}

	fd = open(full_filename, O_RDONLY);
	if (fstat(fd, &s) < 0) {
		lprintf(LOG_DEBUG, "%s: Cannot stat %s\n",__func__, full_filename);
		goto flashrom_read_exit_2;
	}

	*buf = mosys_malloc(s.st_size);
	if (read(fd, *buf, s.st_size) < 0) {
		lperror(LOG_DEBUG, "%s: Unable to read image");
		free(*buf);
		goto flashrom_read_exit_2;
	}

	rc = s.st_size;
flashrom_read_exit_2:
	for (i = 0; args[i] != NULL; i++)
	  free(args[i]);
flashrom_read_exit_1:
	unlink(full_filename);
flashrom_read_exit_0:
	return rc;
}

int flashrom_write_by_name(size_t size, uint8_t *buf,
                  enum programmer_target target, const char *region)
{
	int fd, written, rc = -1;
	const char *path;
	char filename[] = "flashrom_XXXXXX";
	char full_filename[PATH_MAX];
	char region_file[MAX_ARRAY_SIZE];
	char *args[MAX_ARRAY_SIZE];
	int i = 0;

	if (!region)
		goto flashrom_write_exit_0;

	if ((path = flashrom_path()) == NULL)
		goto flashrom_write_exit_0;
	args[i++] = strdup(path);

	if ((i += append_programmer_arg(target, i, args)) < 0)
		goto flashrom_write_exit_0;

	if (in_android == 1) {
		/* In Android, no tmp, but /data is writable */
		strcpy(full_filename, "/data/");
	} else {
		strcpy(full_filename, "/tmp/");
	}
	strcat(full_filename, filename);
	if (mkstemp(full_filename) == -1) {
		lperror(LOG_DEBUG,
			"Unable to make temporary file for flashrom");
		goto flashrom_write_exit_0;
	}

	fd = open(full_filename, O_WRONLY);
	if (fd < 0) {
		lprintf(LOG_DEBUG, "%s: Couldn't open %s: %s\n", __func__,
			full_filename, strerror(errno));
		goto flashrom_write_exit_0;
	}
	written = write(fd, buf, size);
	if (written < 0) {
		lprintf(LOG_DEBUG, "%s: Couldn't write to %s\n", __func__,
			full_filename);
		goto flashrom_write_exit_0;
	}
	if (written != size) {
		lprintf(LOG_DEBUG, "%s: Incomplete write to %s\n", __func__,
			full_filename);
		goto flashrom_write_exit_0;
	}

	args[i++] = strdup("-i");
	strcpy(region_file, region);
	strcat(region_file, ":");
	strcat(region_file, full_filename);
	args[i++] = strdup(region_file);
	args[i++] = strdup("-w");
	args[i++] = strdup("--fast-verify");
	args[i++] = NULL;

	if (do_flashrom(path, args) < 0) {
		lprintf(LOG_DEBUG, "Unable to write region \"%s\"\n", region);
		goto flashrom_write_exit_1;
	}

	rc = written;

flashrom_write_exit_1:
	for (i = 0; args[i] != NULL; i++)
		free(args[i]);
flashrom_write_exit_0:
	unlink(full_filename);
	return rc;
}
