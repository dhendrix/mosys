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
 * @style:      print style
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int kv_pair_print_to_file(FILE* fp, struct kv_pair *kv_list,
		enum kv_pair_style style)
{
	struct kv_pair *kv_ptr;
	int single_found = 0;

	for (kv_ptr = kv_list; kv_ptr != NULL; kv_ptr = kv_ptr->next) {
		if (!kv_ptr->key || !kv_ptr->value)
			continue;

		switch (style) {
		case KV_STYLE_PAIR:{
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
			break;
		}

		case KV_STYLE_VALUE:
			fprintf(fp, "%s", kv_ptr->value);
			if (kv_ptr->next)
				fprintf(fp, " | ");
			break;

		case KV_STYLE_LONG:
			fprintf(fp, "%-20s | %s\n", kv_ptr->key,
                                       kv_ptr->value);
			break;

		case KV_STYLE_SINGLE:{
			const char *single_key = kv_get_single_key();
			if (!single_key)
				break;
			if (!strncmp(single_key, kv_ptr->key,
						strlen(kv_ptr->key))) {
				fprintf(fp, "%s", kv_ptr->value);
				single_found = 1;
			}
			break;
		}
		}
	}

	if (style == KV_STYLE_SINGLE && !single_found)
		return -1;

	fprintf(fp, "\n");
	return 0;
}

/*
 * kv_pair_print  -  print a key=value pair list to mosys output
 *
 * @kv_list:    pointer to key=value list
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int kv_pair_print(struct kv_pair *kv_list)
{
	FILE *fp = mosys_get_output_file();
	enum kv_pair_style style = mosys_get_kv_pair_style();
	return kv_pair_print_to_file(fp, kv_list, style);
}

static const char *single_key;

const char *kv_get_single_key(void)
{
	return single_key;
}

void kv_set_single_key(const char *key)
{
	single_key = key;
}
