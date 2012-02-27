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
 * list.c: A simple generic doubly-linked list implementation
 */

#include <stdlib.h>

#include "mosys/alloc.h"
#include "mosys/list.h"
#include "mosys/mosys.h"

struct ll_node *list_head(struct ll_node *node) {
	MOSYS_CHECK(node);
	while (node->prev)
		node = node->prev;
	return node;
}

struct ll_node *list_insert_before(struct ll_node *node, void *data)
{
	struct ll_node *n = malloc(sizeof(struct ll_node));
	n->data = data;

	if (node) {
		n->prev = node->prev;
		if (node->prev)
			node->prev->next = n;
		node->prev = n;
	} else {
		n->prev = NULL;
	}
	n->next = node;
	return n;
}

/* Delete a node from the list with a given head */
struct ll_node *list_delete(struct ll_node *head, struct ll_node *n)
{
	if (n->prev)
		n->prev->next = n->next;
	else
		head = n->next;
	if (n->next) {
		n->next->prev = n->prev;
	}

	free(n);
	return head;
}

/* Delete every node in a list, starting with the head, pointed to by phead */
void list_cleanup(struct ll_node **phead)
{
	struct ll_node *n = NULL;
	struct ll_node *head = *phead;

	while (head) {
		n = head;
		head = head->next;
		free(n);
	}
	*phead = NULL;
}

/* count how many nodes in a list, starting with the head */
int list_count(struct ll_node *head)
{
	int num = 0;

	while (head) {
		num++;
		head = head->next;
	}

	return num;
}

/* Call callabck for each node in list pointed to be head. */
void list_foreach(struct ll_node *head, ll_node_callback callback)
{
	struct ll_node *n;

	if (callback == NULL)
		return;

	while (head) {
		n = head;
		head = head->next;
		callback(n);
	}

	return;
}
