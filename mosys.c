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
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "mosys/alloc.h"
#include "mosys/big_lock.h"
#include "mosys/command_list.h"
#include "mosys/globals.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/output.h"
#include "mosys/platform.h"

static void usage(void)
{
	printf("usage: %s [options] [commands]\n"
                    "\n"
                    "  Options:\n"
                    "    -k            print data in key=value format\n"
                    "    -l            print data in long format\n"
                    "    -v            verbose (can be used multiple times)\n"
                    "    -f            ignore mosys lock\n"
                    "    -t            display command tree\n"
                    "    -s            print supported platform IDs\n"
                    "    -p [id]       force platform id (bypass auto-detection)\n"
                    "    -h            print this help\n"
                    "    -V            print version\n"
                    "\n", PROGRAM);
}

static void sub_list(struct platform_cmd *sub)
{
	struct platform_cmd *_sub;

	if (!sub)
		return;

	printf("  Commands:\n");
	for (_sub = sub->arg.sub; _sub && _sub->name; _sub++) {
		if (_sub->desc)
			printf("    %-12s  %s\n", _sub->name, _sub->desc);
	}
	printf("\n");
}

static int sub_main(struct platform_intf *intf,
		    struct platform_cmd *sub, int argc, char **argv)
{
	struct platform_cmd *cmd;

	if (!intf || !sub)
		return -1;

	switch (sub->type) {
	case ARG_TYPE_GETTER:
	case ARG_TYPE_SETTER:
		if (!sub->arg.func) {
			lprintf(LOG_ERR, "Undefined Function\n");
			return -1;
		}

		/* this is a function, it might have more args */
		if (argc > 0 && strcmp(argv[0], "help") == 0) {
			platform_cmd_usage(sub);
			return 0;
		}

		/* run command handler */
		return sub->arg.func(intf, sub, argc, argv);

	case ARG_TYPE_SUB:
		if (argc == 0) {
			sub_list(sub);
			return -1;
		}

		/* this is a sub-command, we must have some more args */
		if (argc == 0 || strcmp(argv[0], "help") == 0) {
			sub_list(sub);
			return 0;
		}

		/* search for matching sub-command */
		for (cmd = sub->arg.sub; cmd && cmd->name; cmd++) {
			if (strlen(cmd->name) != strlen(argv[0]))
				continue;
			if (strcmp(cmd->name, argv[0]) == 0) {
				lprintf(LOG_DEBUG, "Subcommand %s (%s)\n",
					cmd->name, cmd->desc);
				return sub_main(intf, cmd, argc - 1,
						&(argv[1]));
			}
		}
		break;

	default:
		lprintf(LOG_ERR, "Unknown subcommand type\n");
		return -1;
	}

	lprintf(LOG_WARNING, "Command not found\n\n");
	sub_list(sub);

	return -1;
}

static int intf_main(struct platform_intf *intf, int argc, char **argv)
{
	struct platform_cmd **_sub, *sub;
	int do_list = 0;
	int ret = 0;

	if (!intf || !intf->sub) {
		lprintf(LOG_ERR, "No commands defined for this platform\n");
		return -1;
	}

	if (argc == 0)
		ret = -1;

	if (argc == 0 || strcmp(argv[0], "help") == 0) {
		do_list = 1;
		usage();
		printf("  Commands:\n");
	}

	/* go through subcommand list for this interface */
	for (_sub = intf->sub; _sub && *_sub; _sub++) {

		sub = *_sub;
		if (do_list) {
			/*FIXME: if the intf had a main sub, call sub_list */
			printf("    %-12s  %s\n", sub->name, sub->desc);
			continue;
		}

		lprintf(LOG_DEBUG, "Command: %s (%s)\n", sub->name, argv[0]);

		/* is this sub-command is the one they asked for? */
		if (strlen(sub->name) != strlen(argv[0]))
			continue;
		if (strcmp(sub->name, argv[0]) == 0) {
			lprintf(LOG_DEBUG, "Found command %s (%s)\n",
				sub->name, sub->desc);
			return sub_main(intf, sub, argc - 1, &(argv[1]));
		}
	}

	if (do_list) {
		printf("\n");
		return ret;
	}

	lprintf(LOG_WARNING, "Command not found\n\n");
	return intf_main(intf, 0, NULL);	/* trigger a help listing */
}

#define LOCK_TIMEOUT_SECS 10

int mosys_main(int argc, char **argv)
{
	int rc;
	int argflag;
	int verbose = 0;
	int force_lock = 0;
	int s_opt = 0;
	int showtree = 0;
	char *p_opt = NULL;
	struct platform_intf *intf;
	enum kv_pair_style style = KV_STYLE_VALUE;

	while ((argflag = getopt(argc, argv, "klvftsp:Vh")) > 0) {
		switch (argflag) {
		case 'k':
			style = KV_STYLE_PAIR;
			break;
		case 'l':
			style = KV_STYLE_LONG;
			break;
		case 'v':
			verbose++;
			break;
		case 'f':
			force_lock = 1;
			break;
		case 't':
			showtree = 1;
			break;
		case 's':
			s_opt = 1;
			break;
		case 'p':
			p_opt = mosys_strdup(optarg);	/* FIXME */
			break;
		case 'V':
			printf("mosys version %s\n", VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	mosys_set_kv_pair_style(style);
	if (s_opt) {
		print_platforms();
		exit(EXIT_SUCCESS);
	}

	/*
	 * Init the logging system and the default log output file (stderr).
	 * Anything logged with a level below CONFIG_LOGLEVEL will not be logged
	 * at all. We use the number of "-v" arguments on the commandline as
	 * a bias against the default threshold.  The more times you use
	 * "-v", the lower your logging threshold becomes.
	 */
	mosys_log_init(PROGRAM, CONFIG_LOGLEVEL+verbose, NULL);

	/* try to get lock */
	if (!force_lock && (mosys_acquire_big_lock(LOCK_TIMEOUT_SECS) < 0)) {
		rc = -1;
		goto do_exit_1;
	}

	/* set the global verbosity level */
	mosys_set_verbosity(verbose);

	/* try to identify the platform */
	intf = mosys_platform_setup(p_opt);
	if (!intf) {
		lprintf(LOG_ERR, "Platform not supported\n");
		rc = -1;
		goto do_exit_2;
	}
	lprintf(LOG_DEBUG, "Platform: %s\n", intf->name);

	if (showtree) {
		print_tree(intf);
		rc = 0;
		goto do_exit_3;
	}

	/* FIXME: debug print */
	/* run command */
	rc = intf_main(intf, argc - optind, &(argv[optind]));
	if (rc == -ENOSYS)
		lprintf(LOG_ERR, "Command not supported on this platform\n");

	/* clean up */
do_exit_3:
	mosys_platform_destroy(intf);
do_exit_2:
	mosys_release_big_lock();
do_exit_1:
	mosys_log_halt();

	if (rc < 0)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	mosys_globals_init();
	return mosys_main(argc, argv);
}
