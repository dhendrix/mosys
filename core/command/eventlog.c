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

#include "lib/common.h"
#include "lib/smbios.h"

#include "mosys/log.h"
#include "mosys/kv_pair.h"
#include "mosys/platform.h"

#include "intf/mmio.h"

static int eventlog_smbios_list_callback(struct platform_intf *intf,
                                         struct smbios_log_entry *entry,
                                         void *arg, int *complete)
{
	int *entry_count = arg;
	struct kv_pair *kv;

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

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int eventlog_smbios_list_cmd(struct platform_intf *intf,
                                    struct platform_cmd *cmd,
                                    int argc, char **argv)
{
	int entry_count = 0;

	return smbios_eventlog_foreach_event(
		intf, intf->cb->eventlog->verify_metadata,
		eventlog_smbios_list_callback, &entry_count);
}

static int eventlog_smbios_info_cmd(struct platform_intf *intf,
				    struct platform_cmd *cmd,
				    int argc, char **argv)
{
	struct smbios_table table;
	struct smbios_table_log *log;
	struct kv_pair *kv;

	/* find the SMBIOS log structure */
	if (smbios_find_table(intf, SMBIOS_TYPE_LOG, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0)
		return -1;

	log = &table.data.log;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "length", "%d bytes", log->length);
	kv_pair_fmt(kv, "header_start", "0x%04x", log->header_start);
	kv_pair_fmt(kv, "data_start", "0x%04x", log->data_start);
	kv_pair_fmt(kv, "method", "0x%02x", log->method);
	kv_pair_fmt(kv, "status", "0x%02x", log->status);
	kv_pair_fmt(kv, "valid", "%s", log->status & 1 ? "yes" : "no");
	kv_pair_fmt(kv, "full", "%s", log->status & 2 ? "yes" : "no");
	kv_pair_fmt(kv, "change_token", "0x%08x", log->change_token);
	kv_pair_fmt(kv, "header_format", "0x%02x", log->header_format);
	kv_pair_fmt(kv, "descriptor_num", "%d", log->descriptor_num);
	kv_pair_fmt(kv, "descriptor_len", "%d", log->descriptor_len);

	kv_pair_print(kv);
	kv_pair_free(kv);

	return 0;
}

static int eventlog_smbios_add_cmd(struct platform_intf *intf,
				   struct platform_cmd *cmd,
				   int argc, char **argv)
{
	if (argc < 1) {
		platform_cmd_usage(cmd);
		return -1;
	}

	if (!intf->cb->eventlog || !intf->cb->eventlog->add) {
		return -ENOSYS;
	}

	return intf->cb->eventlog->add(intf, argc, argv);
}

static int eventlog_smbios_clear_cmd(struct platform_intf *intf,
				     struct platform_cmd *cmd,
				     int argc, char **argv)
{
	enum eventlog_clear_type type;

	if (!intf->cb->eventlog || !intf->cb->eventlog->clear) {
		return -ENOSYS;
	}

	if (argc < 1)
		return intf->cb->eventlog->clear(intf, EVENTLOG_CLEAR_100);

	if (0 == strncmp(argv[0], "25", 2))
		type = EVENTLOG_CLEAR_25;
	else if (0 == strncmp(argv[0], "50", 2))
		type = EVENTLOG_CLEAR_50;
	else if (0 == strncmp(argv[0], "75", 2))
		type = EVENTLOG_CLEAR_75;
	else if (0 == strncmp(argv[0], "100", 3))
		type = EVENTLOG_CLEAR_100;
	else if (0 == strncmp(argv[0], "status", 6))
		type = EVENTLOG_CLEAR_STATUS;
	else
		type = EVENTLOG_CLEAR_100;

	return intf->cb->eventlog->clear(intf, type);
}

struct platform_cmd eventlog_smbios_cmds[] = {
	{
		.name	= "info",
		.desc	= "Event Log Information",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = eventlog_smbios_info_cmd }
	},
	{
		.name	= "list",
		.desc	= "List Event Log",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = eventlog_smbios_list_cmd }
	},
	{
		.name	= "add",
		.desc	= "Add entry to Event Log",
		.usage	= "<event>",
		.type	= ARG_TYPE_GETTER,
		.arg	= { .func = eventlog_smbios_add_cmd }
	},
	{
		.name	= "clear",
		.desc	= "Clear Event Log",
		.usage	= "[25 | 50 | 75 | 100]",
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
