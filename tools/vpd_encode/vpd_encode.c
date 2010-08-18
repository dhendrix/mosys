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
 * vpd_encode.c: vpd encoder proof of concept
 * This file will be entirely re-written very soon. It was hacked together
 * and lashed up to the config system in a short time, and it shows.
 * This will be much more modular and extensible in the near-future.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>

#include "mosys/log.h"

#include "lib/vpd.h"

#include "lib_vpd_encode.h"

#ifndef CONFIG_VPD_OUTFILE
#define CONFIG_VPD_OUTFILE	"vpd.bin"
#endif

static void usage(void)
{
	printf("usage: %s [options] [commands]\n"
                    "\n"
                    "  Options:\n"
                    "    -v            verbose (can be used multiple times)\n"
                    "    -h            print this help\n"
                    "    -V            print version\n"
                    "\n", PROGRAM);
}

int main(int argc, char *argv[])
{
	int fd, rc = EXIT_SUCCESS;
	char outfile[] = CONFIG_VPD_OUTFILE;
	int argflag;
	int verbose = 0;
	struct vpd_entry *eps = NULL;
	uint8_t *table = NULL;		/* the structure table */
	int table_len = 0, num_structures = 0;

	while ((argflag = getopt(argc, argv, "vh")) > 0) {
		switch (argflag) {
		case 'v':
			verbose++;
			break;
		case 'V':
			printf("%s version %s\n", PROGRAM, VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		default:
			break;
		}
	}

	mosys_log_init(PROGRAM, CONFIG_LOGLEVEL+verbose, NULL);

	fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "unable to open file \"%s\": %s\n",
				outfile, strerror(errno));
		rc = EXIT_FAILURE;
		goto do_exit_1;
	}

	/*
	 * All structures belonging to the structure table are created first,
	 * followed by the entry point structure.
	 */
	
	table_len = vpd_append_type0(1, &table, table_len);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;

	table_len = vpd_append_type1(1, &table, table_len);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;

#ifdef CONFIG_BUILD_BBP0
	table_len = vpd_append_type241(0, &table, table_len,
	                               CONFIG_BBP0_UUID,
	                               CONFIG_BBP0_OFFSET,
	                               CONFIG_BBP0_SIZE,
	                               CONFIG_BBP0_VENDOR,
	                               CONFIG_BBP0_DESCRIPTION,
	                               CONFIG_BBP0_BLOB_VARIANT);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;
#endif

#ifdef CONFIG_BUILD_BBP1
	table_len = vpd_append_type241(1, &table, table_len,
	                               CONFIG_BBP1_UUID,
	                               CONFIG_BBP1_OFFSET,
	                               CONFIG_BBP1_SIZE,
	                               CONFIG_BBP1_VENDOR,
	                               CONFIG_BBP1_DESCRIPTION,
	                               CONFIG_BBP1_BLOB_VARIANT);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;
#endif

	table_len = vpd_append_type127(1, &table, table_len);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;

	/* 
	 * create the entry point structure last since it contains information
	 * about the size of the structure table and number of structures within
	 * the table
	 */
	eps = vpd_create_eps(table_len, num_structures);

	/* for now we will simply write the entry point followed by the
	   structure table */
	lprintf(LOG_DEBUG, "entry point struct: %p, length: %#lx\n",
	        eps, eps->entry_length);
	if (write(fd, eps, eps->entry_length) != eps->entry_length) {
		printf("failed to write %s: %s\n", outfile, strerror(errno));
		rc = EXIT_FAILURE;
		goto do_exit_2;
	}
	lprintf(LOG_DEBUG, "table: %p, length: %#lx\n",
	        table, (size_t)table_len);
	if (write(fd, table, table_len) != table_len) {
		printf("failed to write %s: %s\n", outfile, strerror(errno));
		rc = EXIT_FAILURE;
		goto do_exit_2;
	}

	printf("successfully wrote %s\n", outfile);

do_exit_2:
	free(eps);
	free(table);
do_exit_1:
	close(fd);
	mosys_log_halt();
	return rc;
}
