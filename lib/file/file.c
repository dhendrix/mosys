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

#include "mosys/alloc.h"
#include "mosys/log.h"

#include "mosys/list.h"

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
	if (d == NULL) {		/* Probably just ENOTDIR */
		if (errno == EOVERFLOW)	{	/* http://b/issue?id=972812 */
			lprintf(LOG_DEBUG, "%s: readdir(3) error on %s: %s\n",
					__func__, root, strerror(errno));
		}
	}

	closedir(dp);
	return *list;
}
