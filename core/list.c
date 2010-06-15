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
 * list.c: A simple generic doubly-linked list implementation
 */

#include <stdlib.h>

#include "mosys/alloc.h"
#include "mosys/common.h"
#include "mosys/list.h"

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
