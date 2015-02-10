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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "lib/elog_smbios.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"

static int eventlog_smbios_list_callback(struct platform_intf *intf,
                                         struct smbios_log_entry *entry,
                                         void *arg, int *complete)
{
	int *entry_count = arg;
	int rc;
	struct kv_pair *kv;

	if (entry->length == 0) {
		lprintf(LOG_ERR, "Zero-length eventlog entry detected.\n");
		*complete = 1;
		return -1;
	}

	/* verify entry checksum on OEM types */
	if (entry->type >= SMBIOS_EVENT_TYPE_OEM &&
	    intf->cb->eventlog->verify &&
	    !intf->cb->eventlog->verify(intf, entry)) {
		return 0;
	}

	/* see if there is a multi event handler */
	if (intf->cb->eventlog->print_multi) {
		int count = intf->cb->eventlog->print_multi(intf, entry,
							    *entry_count);
		if (count > 0) {
			/* event was handled and 'count' events were added */
			*entry_count += count;
			return 0;
		}
	}

	/* start of list */
	kv = kv_pair_new();

	/* print the record number */
	kv_pair_fmt(kv, "entry", "%d", *entry_count);

	*entry_count += 1;

	/* print the timestamp */
	smbios_eventlog_print_timestamp(intf, entry, kv);

	/* look for a custom print_type handler */
	if (intf->cb->eventlog->print_type == NULL ||
	    intf->cb->eventlog->print_type(intf, entry, kv) == 0) {
		/*
		 * not handled by custom print_type handler
		 * FIXME: pass "Unknown event" to val2str.
		 */
		const char *type = smbios_get_event_type_string(entry);
		kv_pair_add(kv, "type", type ? type : "Unknown");
	}

	/* look for a custom print_data handler */
	if (intf->cb->eventlog->print_data)
		intf->cb->eventlog->print_data(intf, entry, kv);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int eventlog_smbios_list_cmd(struct platform_intf *intf,
                                    struct platform_cmd *cmd,
                                    int argc, char **argv)
{
	int entry_count = 0;

	return smbios_eventlog_foreach_event(
		intf, intf->cb->eventlog->verify_header,
		eventlog_smbios_list_callback, &entry_count);
}

static int eventlog_smbios_add_cmd(struct platform_intf *intf,
				   struct platform_cmd *cmd,
				   int argc, char **argv)
{
	enum smbios_log_entry_type type;
	size_t data_size = 0;
	uint8_t *data = NULL;
	int res;

	if (!intf->cb->eventlog || !intf->cb->eventlog->add) {
		errno = ENOSYS;
		return -1;
	}

	if (argc < 1 || argc > 2) {
		platform_cmd_usage(cmd);
		errno = EINVAL;
		return -1;
	}

	type = strtol(argv[0], NULL, 0);

	if (argc == 2) {
		size_t len = strlen(argv[1]);
		char byte[3] = { 0, 0, 0 };
		char *endptr;
		int i;

		if (len % 2) {
			platform_cmd_usage(cmd);
			errno = EINVAL;
			return -1;
		}

		data_size = len / 2;
		data = mosys_malloc(data_size);
		for (i = 0; i < data_size; i++) {
			byte[0] = *argv[1]++;
			byte[1] = *argv[1]++;
			data[i] = strtol(byte, &endptr, 0);
			if (endptr != &byte[2]) {
				free(data);
				platform_cmd_usage(cmd);
				errno = EINVAL;
				return -1;
			}
		}
	}

	res = intf->cb->eventlog->add(intf, type, data_size, data);

	free(data);
	return res;
}

static int eventlog_smbios_clear_cmd(struct platform_intf *intf,
				     struct platform_cmd *cmd,
				     int argc, char **argv)
{
	if (!intf->cb->eventlog || !intf->cb->eventlog->clear) {
		errno = ENOSYS;
		return -1;
	}

	return intf->cb->eventlog->clear(intf);
}

struct platform_cmd eventlog_smbios_cmds[] = {
	{
		.name	= "list",
		.desc	= "List Event Log",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = eventlog_smbios_list_cmd }
	},
	{
		.name	= "add",
		.desc	= "Add entry to Event Log",
		.usage	= "<event type> [event data]\n\n"
			  "event type is number, event data is a "
			  "series of bytes",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = eventlog_smbios_add_cmd }
	},
	{
		.name	= "clear",
		.desc	= "Clear Event Log",
		.type	= ARG_TYPE_SETTER,
		.arg	= { .func = eventlog_smbios_clear_cmd }
	},
	{ NULL }
};

struct platform_cmd cmd_eventlog = {
	.name	= "eventlog",
	.desc	= "Event Log",
	.type	= ARG_TYPE_SUB,
	.arg	= { .sub = eventlog_smbios_cmds }
};
