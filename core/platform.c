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
 * platform.c: platform interface routines
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosys/alloc.h"
#include "mosys/callbacks.h"
#include "mosys/globals.h"
#include "mosys/intf_list.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"
#include "mosys/output.h"

#include "lib/string.h"

/*
 * mosys_platform_setup  -  identify platform, setup interfaces and commands
 *
 * @p_opt:	Optional platform name given to bypass auto-detection
 * 
 * returns pointer to identified platform interface
 * returns NULL if platform not identified or other error
 */
struct platform_intf *mosys_platform_setup(const char *p_opt)
{
	struct platform_intf **_intf, *intf;
	struct platform_intf *ret = NULL;
	int intf_found = 0;

	/* attempt to probe platform name and match it to an interface */
	for (_intf = platform_intf_list; _intf && *_intf; _intf++) {
		intf = *_intf;

		lprintf(LOG_DEBUG, "Checking platform %s\n", intf->name);

		/* use common operations by default */
		intf->op = &platform_common_op;

		/* platform name specified by user */
		if (p_opt) {
			if (strlfind(p_opt, &intf->id_list[0], 0)) {
				intf_found = 1;
				break;
			} else {
				continue;
			}
		}

		/* auto-detect */
		if (intf->probe && intf->probe(intf)) {
			lprintf(LOG_DEBUG, "Platform %s found (via "
				"probing)\n", intf->name);
			intf_found = 1;
			break;
		}
	}

	if (!intf_found && p_opt)
		goto mosys_platform_setup_exit;

	/* call platform-specific setup if found */
	if (intf->setup && intf->setup(intf) < 0)
		goto mosys_platform_setup_exit;

	/* prepare interface operations */
	if (intf_op_setup(intf) < 0) {
		if (intf->destroy)
			intf->destroy(intf);
		intf_op_destroy(intf);
		goto mosys_platform_setup_exit;
	}

	/* call platform-specific post-setup if found */
	if (intf->setup_post &&
	    intf->setup_post(intf) < 0) {
		if (intf->destroy)
			intf->destroy(intf);
		goto mosys_platform_setup_exit;
	}

	if (!intf_found) {
		/* default platform will be used */
		lprintf(LOG_WARNING, "Unable to auto-detect platform. Limited "
		                     "functionality only.\n");
	}
	ret = intf;

mosys_platform_setup_exit:
	return ret;
}

/*
 * mosys_platform_destroy  -  clean up platform interface
 *
 * @intf:	platform interface
 */
void mosys_platform_destroy(struct platform_intf *intf)
{
	/* Call the destroy callbacks and cleanup afterwards. */
//	invoke_destroy_callbacks();
//	cleanup_destroy_callbacks();

	if (intf) {
		/* cleanup interface */
		if (intf->destroy)
			intf->destroy(intf);

		/* cleanup interface operations */
		intf_op_destroy(intf);

	}
}

/*
 * platform_cmd_usage  -  print usage text for command
 *
 * @cmd:	command pointer
 */
void platform_cmd_usage(struct platform_cmd *cmd)
{
	printf("usage: %s %s\n\n", cmd->name, cmd->usage ? : "");
}

/*
 * tree_sub -	subcommand handler for print_tree
 *
 * @intf:	platform interface
 * @cmd:	command to iterate thru
 * @root:	root number
 * @branch:	branch number (also indicates indentation level)
 * @str:	string with full command name
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure (or end of command hierarchy)
 */
static int tree_subcommand(struct platform_intf *intf,
			   struct platform_cmd *cmd,
			   int root, int branch, char *str)
{
	struct platform_cmd *sub;
	char *tabs;
	int leaf;

	branch++;
	tabs = mosys_malloc(branch + 1);
	memset(tabs, '\t', branch);
	tabs[branch] = '\0';

	for (sub = cmd->arg.sub, leaf = 1; sub->name != NULL; sub++, leaf++) {
		printf("%s", tabs);

		if (sub->type == ARG_TYPE_SUB) {
			int pos = 0, len = 0;

			if (mosys_get_verbosity() > 0) {
				printf("[branch %d:%d] %s ", root, branch, str);

				/* add full command info */
				pos = strlen(str);
				len = strlen(sub->name) + 2;
				snprintf(str + pos, len, " %s", sub->name);
			}

			printf("%s\n", sub->name);

			/* continue descending into the hierarchy */
			tree_subcommand(intf, sub, root, branch, str);

			if (mosys_get_verbosity() > 0) {
				memset(str + pos, 0, len);
			}
		} else if (sub->type == ARG_TYPE_GETTER) {
			if (mosys_get_verbosity() > 0) {
				if (branch == 1) {
					printf("[leaf %d:%d] %s ",
						root, leaf, str);
				} else {
					printf("[leaf %d:%d:%d] %s ",
						root, branch, leaf, str);
				}
			}
			printf("%s\n", sub->name);
		} else if (sub->type == ARG_TYPE_SETTER) {
			if (mosys_get_verbosity() > 0) {
				if (branch == 1) {
					printf("[flur %d:%d] %s ",
						root, leaf, str);
				} else {
					printf("[flur %d:%d:%d] %s ",
						root, branch, leaf, str);
				}
			}
			printf("%s\n", sub->name);
		} else {
			// not a subcommand or function?!?
			return -1;
		}

	}

	branch--;
	free(tabs);
	return 0;
}

/*
 * print_tree - print command tree for this platform
 *
 * @intf:	platform interface
 */
void print_tree(struct platform_intf *intf)
{
	int root;
	char str[64];

	for (root = 0; intf->sub[root] != NULL; root++) {
		if (mosys_get_verbosity() > 0) {
			printf("[root %d] ", root);
			snprintf(str, sizeof(str), "mosys %s",
						   intf->sub[root]->name);
		} else {
			snprintf(str, sizeof(str), "%s", intf->sub[root]->name);
		}
		printf("%s\n", str);

		if (tree_subcommand(intf, intf->sub[root], root, 0, str) < 0)
			lprintf(LOG_DEBUG, "tree walking failed", str);
	}
}

/*
 * print_platforms - print platform ids
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int print_platforms() {
	struct platform_intf **_intf, *intf;
	const char **id;

	/* go through all supported interfaces */
	for (_intf = platform_intf_list; _intf && *_intf; _intf++) {
		intf = *_intf;

		if (intf->type == PLATFORM_DEFAULT)
			continue;

		for (id = intf->id_list; id && *id; id++) {
			struct kv_pair *kv;

			kv = kv_pair_new();
			kv_pair_add(kv, "id", *id);
			kv_pair_print(kv);
			kv_pair_free(kv);
		}
	}

	return 0;
}
