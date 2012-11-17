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
 * file.c: implementations some useful functions for files and directories
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>	/* for PATH_MAX */

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/list.h"
#include "mosys/log.h"

#include "lib/file.h"

/*
 * file_open  -  Generic file open function
 *
 * @file:       file path and name
 * @rw:         read=0, write=1
 *
 * returns valid file descriptor on success
 * returns <0 to indicate failure
 */
int file_open(const char *file, int rw)
{
	struct stat st1, st2;
	int fd;

	/* verify existance */
	if (lstat(file, &st1) < 0) {
		if (rw == FILE_WRITE) {
			/* does not exist, ok to create */
			fd = open(file, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
			if (fd == -1) {
				lperror(LOG_DEBUG,
					"Unable to open file %s for write",
					file);
				return -1;
			}
			/* created ok, now return the descriptor */
			return fd;
		} else {
			lprintf(LOG_DEBUG, "File %s does not exist\n", file);
			return -1;
		}
	}

	if (rw == FILE_READ) {
		/* on read skip the extra checks */
		fd = open(file, O_RDONLY);
		if (fd == -1) {
			lperror(LOG_NOTICE, "Unable to open file %s", file);
			return -1;
		}
		return fd;
	}

	/* it exists - only regular files and char devices */
	if (S_ISREG(st1.st_mode) == 0 && S_ISCHR(st1.st_mode) == 0) {
		lprintf(LOG_NOTICE, "File %s has invalid mode: %d\n",
			file, st1.st_mode);
		return -1;
	}

	/* allow only files with 1 link (itself) */
	if (st1.st_nlink != 1) {
		lprintf(LOG_NOTICE, "File %s has invalid link count: %d != 1\n",
			file, (int)st1.st_nlink);
		return -1;
	}

	fd = open(file, O_RDWR);
	if (fd == -1) {
		lperror(LOG_NOTICE, "Unable to open file %s", file);
		return -1;
	}

	/* stat again */
	if (fstat(fd, &st2) < 0) {
		lperror(LOG_NOTICE, "Unable to stat file %s", file);
		close(fd);
		return -1;
	}

	/* verify inode */
	if (st1.st_ino != st2.st_ino) {
		lprintf(LOG_NOTICE, "File %s has invalid inode: %d != %d\n",
			file, st1.st_ino, st2.st_ino);
		close(fd);
		return -1;
	}

	/* verify owner */
	if (st1.st_uid != st2.st_uid) {
		lprintf(LOG_NOTICE, "File %s has invalid user id: %d != %d\n",
			file, st1.st_uid, st2.st_uid);
		close(fd);
		return -1;
	}

	/* verify inode */
	if (st2.st_nlink != 1) {
		lprintf(LOG_NOTICE, "File %s has invalid link count: %d != 1\n",
			file, st2.st_nlink);
		close(fd);
		return -1;
	}

	return fd;
}

/*
 * scanft - Scan filetree. A file tree walker for finding files containing an
 * optional string at the beginning. The caller should be specific enough with
 * root and symdepth arguments to avoid finding duplicate information
 * (Especially in sysfs).
 *
 * @list:	Double pointer to list where path information will be stored
 * @root:	Where to begin search
 * @filename:	Name of file to search for
 * @str:	Optional NULL terminated string to check at the beginning
 * 		of the file
 * @maxdepth:	Maximum directory depth to follow. A negative value means
 *		follow indefinitely. Zero means do not descend.
 * @symdepth:	Maximum depth of symlinks to follow. A negative value means
 * 		follow indefinitely. Zero means do not follow symlinks.
 * 
 * returns pointer to list to indicate success
 * returns NULL to indicate failure
 */
struct ll_node *scanft(struct ll_node **list,
		       const char *root, const char *filename,
		       const char *str, int maxdepth, int symdepth)
{
	DIR *dp;
	struct dirent *d;
	struct stat s;
	int do_descend = 0;

	if (lstat(root, &s) < 0) {
		lprintf(LOG_DEBUG, "%s: Error stat'ing %s: %s\n",
				__func__, root, strerror(errno));
		return NULL;
	}

	if (S_ISLNK(s.st_mode)) {
		if (symdepth == 0)	/* Leaf has been reached */
			return NULL;
		else if (symdepth > 0)	/* Follow if not too deep in */
			symdepth--;	
	}

	if (maxdepth != 0)
		do_descend = 1;
	if (maxdepth > 0)
		maxdepth--;

	if ((dp = opendir(root)) == NULL) 
		return NULL;

	while ((d = readdir(dp))) {
		char newpath[PATH_MAX];

		/* Skip "." and ".." */
		if (!(strncmp(d->d_name, ".", 1)) ||
		    !(strncmp(d->d_name, "..", 2)))
			continue;

		snprintf(newpath, sizeof(newpath), "%s/%s", root, d->d_name);

		if (!strncmp(d->d_name, filename, strlen(filename))) {
			if (str) {
				FILE *fp;
				char *tmp;
				int len;

				len = strlen(str);
				tmp = mosys_malloc(len);

				if ((fp = fopen(newpath, "r")) == NULL) {
					lprintf(LOG_DEBUG,
						"Error opening %s: %s\n",
						newpath, strerror(errno));
				}
				if (fread(tmp, 1, len, fp) < 1) {
					lprintf(LOG_DEBUG,
						"Error reading %s: %s\n",
						newpath, strerror(ferror(fp)));
				}
				fclose(fp);

				if (!strncmp(str, tmp, len)) {
					*list = list_insert_before(*list,
						 mosys_strdup(newpath));
				}

				free(tmp);
			} else {
				*list = list_insert_before(*list,
					 mosys_strdup(newpath));
			}
		}

		if (do_descend)
			scanft(list, newpath, filename, str, maxdepth, symdepth);
	}

	closedir(dp);
	return *list;
}

static void scanft_free_node(struct ll_node *node)
{
	free(node->data);
}

/*
 * scanft_list_cleanup: clean up the memory of the list generated by scanft
 *                      and delete every node in the list,
 *                      starting with the head, pointed to by phead
 *
 * @phead:      pointer to head of the list which is generated by scanft
 */
void scanft_list_cleanup(struct ll_node **phead)
{
	list_foreach(*phead, scanft_free_node);
	list_cleanup(phead);
}

/*
 * sysfs_lowest_smbus: find lowest numbered smbus device matching name in sysfs
 *
 * @path:	sysfs path to start at
 * @name:	name of device to match
 *
 * returns bus number if successful
 * returns <0 to indicate failure
 */
int sysfs_lowest_smbus(const char *path, const char *name)
{
	struct ll_node *sysfs_ll = NULL, *tmp;
	unsigned int sysfs_lowbus = ~0;
	int ret = -1, found_bus_offset = 0;

	/* populate linked list nodes with all sysfs sub-directories
	 * containing a named device entry matching our criteria */
	scanft(&sysfs_ll, path, "name", name, -1, 1);
	if (!sysfs_ll) {
		lprintf(LOG_DEBUG, "%s: sysfs entry \"%s\" not found\n",
		        __func__, name);
		return -1;
	}

	if (list_count(sysfs_ll) > 1) {
		lprintf(LOG_DEBUG, "%s: multpiple matches for \"%s\" found\n",
		        __func__, name);
	}

	/* find lowest bus as reported by sysfs in the form of
	 * /sys/bus/i2c/devices/i2c-<bus> */
	for (tmp = sysfs_ll; tmp; tmp = tmp->next) {
		char *pos;
		unsigned int bus;

		lprintf(LOG_DEBUG, "%s: scanning %s\n",
		        __func__, (char *)tmp->data);
		pos = strstr((const char *)tmp->data, "i2c-");
		if (!pos)
			continue;

		errno = 0;
		bus = strtol(pos + strlen("i2c-"), NULL, 10);
		if (!errno && (bus < sysfs_lowbus)) {
			sysfs_lowbus = bus;
			found_bus_offset = 1;
		}
	}

	scanft_list_cleanup(&sysfs_ll);
	if (found_bus_offset)
		ret = sysfs_lowbus;
	return ret;
}
