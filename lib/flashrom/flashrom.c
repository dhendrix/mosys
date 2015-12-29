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
#include <fmap.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "mosys/alloc.h"
#include "mosys/big_lock.h"
#include "mosys/globals.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include <lib/eeprom.h>
#include "lib/flashrom.h"
#include "lib/math.h"

#define MAX_ARRAY_SIZE 256

static int in_android = 0;

enum pipe_direction {
	PIPE_IN,
	PIPE_OUT,
	PIPE_NONE,
};

struct pipe {
	enum pipe_direction direction;
	int fd;		/* file descriptor to attach to end of pipe */
	int *pipefd;	/* array of 2 ints that will be passed into pipe() */
	char *buf;	/* buffer to read/write data */
	int size;	/* size of buffer */
};

/*
 * do_cmd - Execute a command and optionally pipe output to/from child process
 *
 * @cmd:	Command to execute.
 * @argv:	Arguments. By convention, argv[0] is cmd.
 * @pipes:	Array of pipes to set up.
 * @num_pipes:	Number of pipes to set up.
 *
 * For pipes where the parent reads and child writes, the caller is responsible
 * for initializing the buffer (e.g. zeroing it out). No terminator will be
 * added by the child since that may alter the content undesirably.
 *
 * When the parent reads from the child, the size member of the pipe struct will
 * be considered a maximum (we may read fewer bytes than specified). When the
 * parent writes to the child, the size member will specify the exact number of
 * bytes which must be written.
 *
 * returns -1 to indicate error, 0 to indicate success
 */
static int do_cmd(const char *cmd, char *const *argv,
			struct pipe pipes[], int num_pipes)
{
	int rc = 0;
#if defined(CONFIG_USE_IPC_LOCK)
	int re_acquire_lock = 1;
#endif
	int pid = -1;
	int status = 0;
	int i;
	int null_fd;
	char dev_null[PATH_MAX];

	snprintf(dev_null, sizeof(dev_null),
			"%s/dev/null", mosys_get_root_prefix());
	null_fd = open(dev_null, O_WRONLY);
	if (null_fd < 0)
		return -1;

	for (i = 0; i < num_pipes; i++) {
		if (pipes[i].direction == PIPE_NONE)
			continue;

		if (pipe(pipes[i].pipefd) < 0) {
			lperror(LOG_DEBUG, "Failed to create pipe");
			return -1;
		}
	}

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
		if (num_pipes > 0) {
			for (i = 0; i < num_pipes; i++) {
				if (pipes[i].direction == PIPE_IN) {
					/* parent reads / child writes */
					close(pipes[i].pipefd[0]);
					dup2(pipes[i].pipefd[1], pipes[i].fd);
				} else if (pipes[i].direction == PIPE_OUT) {
					/* parent writes / child reads */
					close(pipes[i].pipefd[1]);
					dup2(pipes[i].pipefd[0], pipes[i].fd);
				} else if (pipes[i].direction == PIPE_NONE) {
					dup2(null_fd, pipes[i].fd);
				}
			}
		} else {
			/* default: pipe stdio to /dev/null */
			dup2(null_fd, fileno(stdin));
			dup2(null_fd, fileno(stdout));
			dup2(null_fd, fileno(stderr));
		}

		execv(cmd, argv);

		lperror(LOG_ERR, "%s: Failed to run %s", __func__, cmd);
		rc = -1;
	} else { /* parent */
		for (i = 0; i < num_pipes; i++) {
			if (pipes[i].direction == PIPE_IN) {
				/* parent reads / child writes */
				close(pipes[i].pipefd[1]);
			} else if (pipes[i].direction == PIPE_OUT) {
				/* parent writes / child reads */
				close(pipes[i].pipefd[0]);
			}
		}

		if (waitpid(pid, &status, 0) > 0) {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					lprintf(LOG_ERR,
						"Child process returned %d.\n",
						WEXITSTATUS(status));
					rc = -1;
				}
			} else {
				lprintf(LOG_ERR, "Child process did not exit "
						"normally.\n");
				rc = -1;
			}
		} else {
			lprintf(LOG_ERR, "waitpid() returned error.\n");
			rc = -1;
		}

		for (i = 0; i < num_pipes; i++) {
			int n;

			errno = 0;
			if (pipes[i].direction == PIPE_IN) {
				n = read(pipes[i].pipefd[0], pipes[i].buf,
							pipes[i].size);
				if (errno) {
					lperror(LOG_ERR, "Failed to read %d "
							"bytes from pipe",
							pipes[i].size);
					rc = -1;
				} else {
					lprintf(LOG_DEBUG, "%s: Read %d bytes "
						"from pipe.\n", __func__, n);
				}

				close(pipes[i].pipefd[0]);
			} else if (pipes[i].direction == PIPE_OUT) {
				n = write(pipes[i].pipefd[1], pipes[i].buf,
							pipes[i].size);
				if (n != pipes[i].size) {
					lperror(LOG_ERR, "%s: Failed to write "
						"%d bytes to pipe", __func__,
						pipes[i].size);
					rc = -1;
				} else {
					lprintf(LOG_DEBUG, "%s: Wrote %d bytes "
						"to pipe.\n", __func__, n);
				}

				close(pipes[i].pipefd[1]);
			}
		}
	}

#if defined(CONFIG_USE_IPC_LOCK)
        /* try to get lock */
        if (re_acquire_lock && (mosys_acquire_big_lock(50000) < 0)) {
		lprintf(LOG_DEBUG, "%s: could not re-acquire lock\n", __func__);
		rc = -1;
        }
#endif
	close(null_fd);
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

/* returns pointer to string containing flashrom path if successful,
   returns NULL otherwise */
static const char *flashrom_path(void)
{
	static char path[PATH_MAX];
	int fd;
	int i = 0;
	struct stat s;
	char android_path[] = "/system/bin/flashrom";
	char which_cmd[] = "/usr/bin/which";
	char *args[MAX_ARRAY_SIZE];
	int stdout_pipefd[2];
	struct pipe pipes[] = {
		{ PIPE_IN, fileno(stdout), stdout_pipefd, path, PATH_MAX },
		{ PIPE_NONE, fileno(stderr) },
	};

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

	/* We're not in Android, try using the `which` command. */
	args[i++] = strdup(which_cmd);
	args[i++] = strdup("flashrom");
	args[i++] = NULL;
	memset(path, 0, sizeof(path));
	if (do_cmd(which_cmd, args, pipes, ARRAY_SIZE(pipes)) < 0) {
		lprintf(LOG_DEBUG, "Unable to determine flashrom path\n");
		return NULL;
	}

	for (i = 0; i < PATH_MAX; i++) {
		if (path[i] == EOF || path[i] == '\n') {
			path[i] = '\0';
			break;
		}
	}

	lprintf(LOG_DEBUG, "%s: path: \"%s\"\n", __func__, path);
	for (i = 0; args[i] != NULL; i++)
		free(args[i]);
	return path;
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

	if (do_cmd(path, args, NULL, 0) < 0)
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

	if (do_cmd(path, args, NULL, 0) < 0) {
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

	if (do_cmd(path, args, NULL, 0) < 0) {
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

int flashrom_read_host_firmware_region(struct platform_intf *intf,
							uint8_t **buf)
{
	int rc, i;
	uint8_t *fmap_buf;
	const char *regions[] = { "COREBOOT", "BOOT_STUB" };

	rc = flashrom_read_by_name(&fmap_buf, HOST_FIRMWARE, "FMAP");
	if (rc > 0) {
		for (i = 0; i < ARRAY_SIZE(regions); i++) {
			if (fmap_find_area((struct fmap *)fmap_buf, regions[i])) {
				rc = flashrom_read_by_name(buf,
						HOST_FIRMWARE, regions[i]);
				break;
			}
		}
		free(fmap_buf);
	} else {
		/* FMAP blob might still be in the ROM but without its own area
		 * defined. Try looking for the firmware regions directly. */
		for (i = 0; i < ARRAY_SIZE(regions); i++) {
			rc = flashrom_read_by_name(buf,
					HOST_FIRMWARE, regions[i]);
			if (rc > 0)
				break;
		}
	}

	return rc;
}

int flashrom_get_rom_size(struct platform_intf *intf,
			enum programmer_target target)
{
	int ret = -1;
	const char *path;
	char *args[MAX_ARRAY_SIZE];
	int i = 0;
	int stdout_pipefd[2];
	size_t stdout_buf_len = 16;
	char stdout_buf[stdout_buf_len];
	struct pipe pipes[] = {
		{ PIPE_IN, fileno(stdout),
			stdout_pipefd, stdout_buf, stdout_buf_len },
		{ PIPE_NONE, fileno(stderr) },
	};

	if ((path = flashrom_path()) == NULL)
		goto flashrom_get_rom_size_exit_0;
	args[i++] = strdup(path);

	if ((i += append_programmer_arg(target, i, args)) < 0)
		goto flashrom_get_rom_size_exit_1;

	args[i++] = strdup("--get-size");
	args[i++] = NULL;

	memset(stdout_buf, 0, sizeof(stdout_buf));
	if (do_cmd(path, args, pipes, ARRAY_SIZE(pipes)) < 0) {
		lprintf(LOG_DEBUG, "Unable to get ROM size\n");
		goto flashrom_get_rom_size_exit_1;
	}

	errno = 0;
	ret = strtol(stdout_buf, NULL, 0);
	if (errno) {
		lperror(LOG_ERR, "Failed to obtain ROM size using flashrom");
		ret = -1;
	}

flashrom_get_rom_size_exit_1:
	for (i = i - 1 ; i > 0; i--)
		free(args[i]);
flashrom_get_rom_size_exit_0:
	return ret;
}
