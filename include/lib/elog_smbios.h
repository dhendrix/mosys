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

#ifndef MOSYS_LIB_EVENTLOG_H__
#define MOSYS_LIB_EVENTLOG_H__

#include <inttypes.h>
#include <sys/types.h>

#include "mosys/platform.h"

#include "lib/smbios_tables.h"

/*
 * SMBIOS eventlog callback definition support.
 */

/* Callback definition for iterating through SMBIOS eventlog events.
 * After each call to the callback the return code is bitwise OR'd with
 * the previous return code.  Also the complete argument is queried to determine
 * if the sequence should continue with the next entry.
 *
 * @intf - platform interface
 * @entry - the current eventlog entry in sequence
 * @arg - optional argument passed in by caller
 * @complete - 1 indicates stop sequence, 0 indicates continuing.
 */
typedef int (*smbios_eventlog_callback)(struct platform_intf *intf,
                                        struct smbios_log_entry *entry,
                                        void *arg, int *complete);

/* Callback definition for an optional verification step of the eventlog
 * metadata when sequencing through each event.
 *
 * @eventlog_header  SMBIOS eventlog header
 *
 * returns 0 on successful verification, < 0 otherwise.
 */
typedef int (*smbios_eventlog_verify_header)(struct elog_header *elog_header);

/* SMBIOS Event Log */
extern struct smbios_eventlog_iterator *smbios_new_eventlog_iterator(
    struct platform_intf *intf, uint8_t *data, size_t length,
    off_t header_offset, off_t data_offset);
extern void smbios_free_eventlog_iterator(
    struct smbios_eventlog_iterator *elog_iter);
extern int smbios_eventlog_iterator_reset(
    struct smbios_eventlog_iterator *elog_iter);
extern struct smbios_log_entry *smbios_eventlog_get_next_entry(
    struct smbios_eventlog_iterator *elog_iter);
extern struct smbios_log_entry *smbios_eventlog_get_current_entry(
    struct smbios_eventlog_iterator *elog_iter);
extern void *smbios_eventlog_get_header(
    struct smbios_eventlog_iterator *elog_iter);
extern const char *smbios_get_event_type_string(struct smbios_log_entry *entry);
extern void smbios_eventlog_print_timestamp(struct platform_intf *intf,
					    struct smbios_log_entry *entry,
					    struct kv_pair *kv);

/*
 * smbios_eventlog_event_time - obtain time of smbios event entry in
 *                              time_t form.
 *
 * @entry - smbios event
 * @time - time_t variable to fill in
 *
 * returns 0 on succes, < 0 on failure
 */
extern int smbios_eventlog_event_time(struct smbios_log_entry *entry,
				      time_t *time);

/*
 * smbios_eventlog_foreach_event - call callback for each event in the SMBIOS
 *                                 eventlog.
 *
 * @intf - platform interface
 * @verify - optional function to call to verify the eventlog metadata
 * @callback - function to call for each log entry
 * @arg - optional argument to pass to callback
 *
 * returns the aggregation (OR) of return codes for each call to callback.
 */
extern int smbios_eventlog_foreach_event(struct platform_intf *intf,
					 smbios_eventlog_verify_header verify,
					 smbios_eventlog_callback callback,
					 void *arg);

#endif /* MOSYS_LIB_SMBIOS_H__ */
