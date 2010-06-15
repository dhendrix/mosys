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
