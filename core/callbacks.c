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
