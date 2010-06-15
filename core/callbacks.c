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
 * callbacks.c: Registration of callbacks for tearing things properly.
 */

#include "mosys/alloc.h"
#include "mosys/list.h"

struct callback {
	void (*func)(void *);
	void *arg;
};

struct ll_node *destroy_callback_head;

/*
 * add_destroy_callback - add a callback function to be called at destroy time
 *
 * @func:   function to be called add destroy time
 * @arg:    argument to pass to func  when func is called
 */
void add_destroy_callback(void (*func)(void *), void *arg)
{
	struct callback *cb;

	if (func == NULL)
		return;

	cb = malloc(sizeof(*cb));

	cb->func = func;
	cb->arg = arg;

	destroy_callback_head = list_insert_before(destroy_callback_head, cb);
}

/*
 * invoke_destroy_callback_single - call the callback pointed to by the data
 *                                  field of the list node.
 *
 * @node:   list node that holds the callback structure
 */
static void invoke_destroy_callback_single(struct ll_node *node)
{
	struct callback *cb;

	cb = node->data;
	cb->func(cb->arg);
}

/*
 * invoke_destroy_callbacks - call every callback contained destroy list
 */
void invoke_destroy_callbacks(void)
{
	list_foreach(destroy_callback_head, invoke_destroy_callback_single);
}

/*
 * cleanup_destroy_callback_single - free the allocated callback structure in
 *                                   the given node.
 *
 * @node:   list node containing the callback structure to free
 */
static void cleanup_destroy_callback_single(struct ll_node *node)
{
	free(node->data);
}

/*
 * cleanup_destroy_callbacks - free all used memory by destroy callbacks
 */
void cleanup_destroy_callbacks(void)
{
	list_foreach(destroy_callback_head, cleanup_destroy_callback_single);
	list_cleanup(&destroy_callback_head);
}
