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
 * kv_pair.c: key-value pair handler functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "mosys/alloc.h"
#include "mosys/globals.h"
#include "mosys/kv_pair.h"

/*
 * globally used kv_pair_style
 */
static enum kv_pair_style mosys_kv_pair_style;

void mosys_set_kv_pair_style(enum kv_pair_style style)
{
	mosys_kv_pair_style = style;
}

enum kv_pair_style mosys_get_kv_pair_style()
{
	return mosys_kv_pair_style;
}


/*
 * kv_pair_new  -  create new key=value pair
 *
 * returns pointer to new key=value pair
 */
struct kv_pair *kv_pair_new(void)
{
	struct kv_pair *kv = mosys_zalloc(sizeof(*kv));
	return kv;
}

/*
 * kv_pair_add  -  add new key=value pair to list
 *
 * @kv_list:    key=value pair list
 * @key:        key string
 * @value:      value string
 *
 * returns pointer to new key=value pair
 * returns NULL to indicate error
 */
struct kv_pair *kv_pair_add(struct kv_pair *kv_list,
			    const char *key, const char *value)
{
	struct kv_pair *kv_new = kv_pair_new();
	struct kv_pair *kv_ptr;

	/* save key=value strings if provided */
	if (key) {
		kv_new->key = mosys_strdup(key);
	}
	if (value) {
		kv_new->value = mosys_strdup(value);
	}

	/* first in the list if no list provided */
	if (kv_list) {
		/* find the end of list */
		for (kv_ptr = kv_list; kv_ptr->next != NULL; kv_ptr = kv_ptr->next) ;

		/* link in the new pair at the end */
		kv_ptr->next = kv_new;
	}

	/* return pointer to the new pair */
	return kv_new;
}

/*
 * kv_pair_add_bool  -  add new boolean kvpair to list
 *
 * @kv_list:    key=value pair list
 * @key:        key string
 * @value:      value
 *
 * returns pointer to new key=value pair
 * returns NULL to indicate error
 */
struct kv_pair *kv_pair_add_bool(struct kv_pair *kv_list,
                                 const char *key, int value)
{
	const char *str;

	if (value) {
		str = "yes";
	} else {
		str = "no";
	}
	return kv_pair_add(kv_list, key, str);
}

/*
 * kv_pair_fmt  -  add key=value pair based on printf format
 *
 * @kv_list:    list of key=value pairs
 * @kv_key:     key string
 * @format:     printf-style format for value input
 * @...:        arguments to format
 *
 * returns pointer to new key=value pair
 * returns NULL to indicate error
 */
struct kv_pair *kv_pair_fmt(struct kv_pair *kv_list,
			    const char *kv_key, const char *format, ...)
{
	char kv_value[KV_PAIR_MAX_VALUE_LEN];
	va_list vptr;

	memset(kv_value, 0, sizeof(kv_value));

	va_start(vptr, format);
	vsnprintf(kv_value, sizeof(kv_value), format, vptr);
	va_end(vptr);

	return kv_pair_add(kv_list, kv_key, kv_value);
}

/*
 * kv_pair_free  -  clean a key=value pair list
 *
 * @kv_list:    pointer to key=value list
 */
void kv_pair_free(struct kv_pair *kv_list)
{
	struct kv_pair *kv_ptr = kv_list;
	struct kv_pair *kv_next;

	while (kv_ptr != NULL) {
		/* free key/value strings */
		if (kv_ptr->key)
			free(kv_ptr->key);
		if (kv_ptr->value)
			free(kv_ptr->value);

		/* free current pair move to next */
		kv_next = kv_ptr->next;
		free(kv_ptr);
		kv_ptr = kv_next;
	}
}

/*
 * kv_pair_print_to_file  -  print a key=value pair list
 *
 * @kv_list:    pointer to key=value list
 */
void kv_pair_print_to_file(FILE* fp, struct kv_pair *kv_list, enum kv_pair_style style)
{
	struct kv_pair *kv_ptr;

	switch (style) {
	case KV_STYLE_PAIR:
		for (kv_ptr = kv_list; kv_ptr != NULL; kv_ptr = kv_ptr->next) {
			if (kv_ptr->key && kv_ptr->value) {
				int i, len = strlen(kv_ptr->value);
				/* need to escape quotes in value */
				fprintf(fp, "%s=\"", kv_ptr->key);
				for (i = 0; i < len; i++) {
					if (kv_ptr->value[i] == '"')
						fprintf(fp, "\\\"");
					else
						fprintf(fp, "%c", kv_ptr->value[i]);
				}
				fprintf(fp, "\" ");
			}
		}
		break;

	case KV_STYLE_VALUE:
		for (kv_ptr = kv_list; kv_ptr != NULL; kv_ptr = kv_ptr->next) {
			if (kv_ptr->value) {
				fprintf(fp, "%s", kv_ptr->value);
				if (kv_ptr->next)
					fprintf(fp, " | ");
			}
		}
		break;

	case KV_STYLE_LONG:
		for (kv_ptr = kv_list; kv_ptr != NULL; kv_ptr = kv_ptr->next) {
			if (kv_ptr->key && kv_ptr->value)
				fprintf(fp, "%-20s | %s\n", kv_ptr->key,
                                        kv_ptr->value);
		}
		break;
	}

	fprintf(fp, "\n");
}

/*
 * kv_pair_print  -  print a key=value pair list to mosys output
 *
 * @kv_list:    pointer to key=value list
 */
void kv_pair_print(struct kv_pair *kv_list)
{
	FILE *fp = mosys_get_output_file();
	enum kv_pair_style style = mosys_get_kv_pair_style();
	kv_pair_print_to_file(fp, kv_list, style);
}
