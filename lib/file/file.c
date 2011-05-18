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
 * @symdepth:	Maximum depth of symlinks to follow. A negative value means
 * 		follow indefinitely. Zero means do not follow symlinks.
 * 
 * returns pointer to list to indicate success
 * returns NULL to indicate failure
 */
struct ll_node *scanft(struct ll_node **list,
		       const char *root, const char *filename,
		       const char *str, int symdepth)
{
	DIR *dp;
	struct dirent *d;
	struct stat s;

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

		scanft(list, newpath, filename, str, symdepth);
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
	scanft(&sysfs_ll, path, "name", name, 1);
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
