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
 * symbol.c: symbol handling
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/string_builder.h"

#include "mosys/alloc.h"
#include "mosys/list.h"
#include "mosys/log.h"
#include "mosys/globals.h"

#include "symbol.h"

static struct ll_node *symbols;

/* for debugging */
#if 0
static void print_symbol(struct ll_node *node)
{
	struct vpd_symbol *symbol;

	if (node->data)
		symbol = node->data;
	else
		return;

	lprintf(LOG_DEBUG, "symbol->name: \"%s\", symbol->value: "
	       "\"%s\"\n", symbol->name, symbol->value);
}
#endif

void free_vpd_symbol(struct vpd_symbol *symbol)
{
	free(symbol->name);
	free(symbol->value);
	free(symbol);
	symbol = NULL;
}

void free_vpd_symbol_node(struct ll_node *node)
{
	struct vpd_symbol *symbol;

	if (!node->data)
		return;

	symbol = (struct vpd_symbol *)node->data;
	free_vpd_symbol(symbol);
}

void cleanup_symtab()
{
	list_foreach(symbols, free_vpd_symbol_node);
	list_cleanup(&symbols);
}

/*
 * parse_symbol - parse a buffer and generate a symbol, if possible
 *
 * @str:	string to parse
 * @len:	length
 *
 * Parse two substrings delimited by equal ('=') sign:
 * First string: The symbol name, beginning with CONFIG_. No spaces,
 *               quotes, or special symbols allowed.
 * Second string: The symbol value, interpreted as follows:
 *     - If enclosed in quotes, value is a string.
 *     - If no quotes and the character is 'y' or 'n', value is
 *       assumed to be booloean.
 *     - If no quotes and character is a number, value is assumed
 *       to be numerical.
 *
 * Example:
 * CONFIG_SYMBOL_STRING="symbol value"
 * CONFIG_SYMBOL_BOOLEAN=y
 * CONFIG_SYMBOL_NUMERICAL=0xdeadbeef
 *
 * returns a newly allocated vpd_symbol if successful
 * returns NULL otherwise
 */
static struct vpd_symbol *parse_symbol(const char *str, size_t len)
{
	struct vpd_symbol *symbol = NULL;
	char *p;
	struct string_builder *sb;
	int index = 0;

	if (!str) {
		lprintf(LOG_DEBUG, "%s: string is NULL\n", __func__);
	}

	if (len < strlen("CONFIG_")) {
		lprintf(LOG_DEBUG, "%s: too few bytes: %lu\n", __func__, len);
		return NULL;
	}

	if (strncmp(str , "CONFIG_", 7)) {
		lprintf(LOG_DEBUG, "%s: string does not begin with "
		        "CONFIG_\n", __func__);
		return NULL;
	}

	p = strchr(str, '=');
	if (!p) {
		lprintf(LOG_DEBUG, "%s: string \"%s\" is not a symbol=value "
		        "pair\n", __func__, str);
		return NULL;
	}

	index = p - str;
	symbol = mosys_zalloc(sizeof(*symbol));
	symbol->name = mosys_malloc(index + 1);
	snprintf(symbol->name, index + 1, "%s", str);

	/* iterate past the '=' and copy the symbol's
	   value one character at a time */
	p++;
	index += sizeof(char);
	sb = new_string_builder();

	if (*p == '"') {
		/* symbol is a string, so we will stop parsing when
		   we encounter the end quote */
		p++;	/* iterate past open quote */
		while (*p != '"' && index < len) {
			string_builder_add_char(sb, *p);
			p++;
			index += sizeof(char);
		}

		if (*p != '"') {
			lprintf(LOG_ERR, "Error parsing \"%s\": "
			        "no end quote found\n", symbol->name);
			free_vpd_symbol(symbol);
			free_string_builder(sb);
			symbol = NULL;
			goto parse_symbol_exit;
		}

		symbol->type = STRING;
	} else if (isdigit(*p)) {
		char *endptr;
		long long int u;

		u = strtoll(p, &endptr, 0);

		/* finish parsing the string for error checking */
		while (!isspace(*p) && index < len) {
			p++;
			index += sizeof(char);
		}

		if (endptr != p) {
			lprintf(LOG_ERR, "Error parsing \"%s\": "
			        "value is non-numeric\n", symbol->name);
			free_vpd_symbol(symbol);
			free_string_builder(sb);
			symbol = NULL;
			goto parse_symbol_exit;
		}

		string_builder_sprintf(sb, "%lld", u);

		symbol->type = NUMERIC;
	} else if (*p == 'y' || *p == 'n') {
		/* symbol is boolean, so we'll stop parsing after
		   this character is read */
		string_builder_add_char(sb, *p);

		/* FIXME: Add sanity checking */
		symbol->type = BOOLEAN;
	}

	symbol->value = mosys_strdup(string_builder_get_string(sb));
	free_string_builder(sb);

	lprintf(LOG_DEBUG, "%s: symbol->name: %s, symbol->value: %s\n",
	        __func__, symbol->name, symbol->value);

parse_symbol_exit:
	return symbol;
}

/*
 * gen_symtab - Parse config file and insert symbols into a table (linked list)
 *
 * Assumes file is in the format:
 * CONFIG_SYMBOL_STRING=value
 *
 * where the value may be a numeric value, a boolean 'y' or 'n' (for yes / no),
 * or a string enclosed in quotation marks ("")
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int gen_symtab(const char *file)
{
	FILE *fp;
	char *line = NULL;
	size_t read;
	size_t len = 0;

	fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr, "unable to open fp file \"%s\": %s\n",
				file, strerror(errno));
		return -1;
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		struct vpd_symbol *symbol = NULL;

		/* line is commented */
		if (line[0] == '#')
			continue;

		if (isspace(line[0]))
			continue;

		symbol = parse_symbol(line, read);
		if (symbol)
			symbols = list_insert_before(symbols, symbol);
	}

	if (line)
		free(line);
	fclose(fp);
	return 0;
}

struct vpd_symbol *lookup_symbol(const char *symbol)
{
	struct ll_node *node;
	struct vpd_symbol *ret = NULL;

	if (symbols)
		node = list_head(symbols);
	else
		return NULL;

	while (node) {
		struct vpd_symbol *sym = node->data;

		if (!sym) {
			lprintf(LOG_DEBUG, "%s: no data for this node???\n");
			continue;
		}

		if (!strcmp(symbol, sym->name)) {
			ret = sym;
			break;
		}

		node = node->next;
	}

	return ret;
}

char *sym2str(const char *symbol)
{
	struct vpd_symbol *s;

	s = lookup_symbol(symbol);
	if (!s)
		return NULL;

	return s->value;
}

int sym2bool(const char *symbol)
{
	struct vpd_symbol *s;
	unsigned long int val = 0;

	s = lookup_symbol(symbol);
	if (!s)
		return 0;

	if (s->type != BOOLEAN)
		return 0;

	if (s->value[0] == 'y') {
		val = 1;
	} else if (s->value[0] == 'n') {
		val = 0;
	} else {
		lprintf(LOG_DEBUG, "%s: Non-boolean value, s->value[0]: %c\n",
			__func__, s->value[0]);
	}

	return val;
}

struct vpd_symbol *update_symbol(const char *str)
{
	struct vpd_symbol *input, *s = NULL;

	/* process input string as a symbol */
	input = parse_symbol(str, strlen(str));
	if (!input)
		return NULL;

	s = lookup_symbol(input->name);
	if (!s) {		/* insert new symbol */
		symbols = list_insert_before(symbols, input);
	} else {		/* edit existing symbol */
		switch(s->type){
		case BOOLEAN:
			s->value = input->value;
			break;
		case NUMERIC:
			s->value = input->value;
			break;
		case STRING:
			free(s->value);
			s->value = mosys_strdup(input->value);
			break;
		default:
			lprintf(LOG_ERR, "cannot insert symbol: %s\n", str);
			break;
		}

		free(input->name);
		free(input->value);
		free(input);
	}

	return s;
}
