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
 * basic doubly-linked list functions
 */

#ifndef MOSYS_LIST_H__
#define MOSYS_LIST_H__

/* A basic list node */
struct ll_node {
	struct ll_node *next;
	struct ll_node *prev;
	void *data;
};

typedef void (*ll_node_callback)(struct ll_node *node);

/*
 * Returns the head node of a list that contains the given node.
 */
extern struct ll_node *list_head(struct ll_node *node);

/*
 * Wrap arbitrary data into a linked list node and insert the newly created
 * node before the requested node in a list.
 *
 * @node: Arbitrary node in target list
 * @data: Data to store in newly created node.
 */
extern struct ll_node *list_insert_before(struct ll_node *node, void *data);

/* Delete a node from the list with a given head */
extern struct ll_node *list_delete(struct ll_node *head, struct ll_node *n);

/* Delete every node in a list, starting with the head, pointed to by phead */
extern void list_cleanup(struct ll_node **phead);

/* count how many nodes in a list, starting with the head */
extern int list_count(struct ll_node *head);

/* Call callabck for each node in list pointed to be head. */
extern void list_foreach(struct ll_node *head, ll_node_callback callback);

#endif /* MOSYS_LIST_H__ */
