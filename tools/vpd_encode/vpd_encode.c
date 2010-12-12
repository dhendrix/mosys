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

#include "mosys/globals.h"
#include "mosys/log.h"

#include "lib/vpd.h"

#include "lib_vpd_encode.h"

#include "symbol.h"

#ifndef CONFIG_VPD_OUTFILE
#define CONFIG_VPD_OUTFILE	"vpd.bin"
#endif

#ifndef VPD_ENCODE_CONFIG
static char *vpd_encode_config = "vpd_encode.config";
#else
static char *vpd_encode_config = VPD_ENCODE_CONFIG;
#endif

static void usage(void)
{
	printf("usage: %s [options] <configfile>\n"
                    "\n"
                    "  Options:\n"
                    "    -u            update (or add) a symbol by supplying a symbol=value pair\n"
                    "    -v            verbose (can be used multiple times)\n"
                    "    -h            print this help\n"
                    "    -V            print version\n"
                    "\n"
		    "  A configuration file can be specified which will\n"
		    "  override values specified at build-time.\n"
		    "\n"
		    "  Symbols may be overridden or added using the -s option.\n"
		    "  Examples:\n"
		    "  Boolean: vpd_encode -s CONFIG_MY_BOOLEAN=y\n"
		    "  Number:  vpd_encode -s CONFIG_MY_NUMBER=0x01234567\n"
		    "  String:  vpd_encode -s 'CONFIG_MY_STRING=\"mystring\"'\n"
		    "  Note: Make sure to enclose double-quotes as necessary for your shell.\n"
		    "\n",
		    PROGRAM);
}

int main(int argc, char *argv[])
{
	int fd, rc = EXIT_SUCCESS;
	char outfile[] = CONFIG_VPD_OUTFILE;
	int argflag;
	int verbose = 0;
	struct vpd_entry *eps = NULL;
	uint8_t *table = NULL;		/* the structure table */
	int table_len = 0;
	uint16_t num_structures = 0;
	struct stat s;
	struct ll_node *user_symbols = NULL;

	while ((argflag = getopt(argc, argv, "vVhu:")) > 0) {
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
		case 'u':
			user_symbols = list_insert_before(user_symbols, optarg);
			break;
		default:
			break;
		}
	}

	/* set up logging and verbosity level */
	mosys_log_init(PROGRAM, CONFIG_LOGLEVEL+verbose, NULL);
	mosys_set_verbosity(verbose);

	if (argv[optind]) {
		printf("setting config as %s\n", argv[optind]);
		vpd_encode_config = argv[optind];
		printf("config: %s\n", vpd_encode_config);
	}
	stat(vpd_encode_config, &s);
	if (S_ISREG(s.st_mode) || S_ISLNK(s.st_mode)) {
		gen_symtab(vpd_encode_config);
	}


	if (user_symbols) {
		struct ll_node *tmp = list_head(user_symbols);

		while (tmp) {
			update_symbol(tmp->data);
			tmp = tmp->next;
		}
	}

#ifdef CONFIG_BUILD_AGZ_VENDOR_VPD_BLOB_V3
	build_agz_vendor_blob(3, CONFIG_AGZ_BLOB_V3_FILENAME);
#endif
#ifdef CONFIG_BUILD_AGZ_VENDOR_VPD_BLOB_V5
	build_agz_vendor_blob(5, CONFIG_AGZ_BLOB_V5_FILENAME);
#endif
	if (sym2bool("CONFIG_BUILD_GOOGLE_VPD_BLOB_V1_1")) {
		build_google_vpd_blob(1.1,
		           sym2str("CONFIG_GOOGLE_BLOB_V1_1_FILENAME"));
	}

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
#ifdef CONFIG_BUILD_VPD_TYPE0_STRUCTURE
	lprintf(LOG_DEBUG, "%s: appending type0 table at structure table"
	                   " offset: 0x%04x\n", __func__, table_len);
	table_len = vpd_append_type0(1, &table, table_len);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;
#endif

#ifdef CONFIG_BUILD_VPD_TYPE1_STRUCTURE
	lprintf(LOG_DEBUG, "%s: appending type1 table at structure table"
	                   " offset: 0x%04x\n", __func__, table_len);
	table_len = vpd_append_type1(1, &table, table_len);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;
#endif

#ifdef CONFIG_BUILD_BBP0
	lprintf(LOG_DEBUG, "%s: appending type241 table at structure table"
	                   " offset: 0x%04x\n", __func__, table_len);
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
	lprintf(LOG_DEBUG, "%s: appending type241 table at structure table"
	                   " offset: 0x%04x\n", __func__, table_len);
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

	lprintf(LOG_DEBUG, "%s: appending type127 table at structure table"
	                   " offset: 0x%04x\n", __func__, table_len);
	table_len = vpd_append_type127(1, &table, table_len);
	if (table_len < 0)
		goto do_exit_2;
	num_structures++;

	/* 
	 * create the entry point structure last since it contains information
	 * about the size of the structure table and number of structures within
	 * the table
	 */
	eps = vpd_create_eps(CONFIG_EPS_VPD_MAJOR_VERSION,
	                     CONFIG_EPS_VPD_MINOR_VERSION,
	                     (uint16_t)table_len,
	                     CONFIG_EPS_STRUCTURE_TABLE_ADDRESS,
#ifdef CONFIG_EPS_NUM_STRUCTURES
	                     CONFIG_EPS_NUM_STRUCTURES
#else
	                     num_structures
#endif
			);

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
	cleanup_symtab();
	close(fd);
	mosys_log_halt();
	return rc;
}
