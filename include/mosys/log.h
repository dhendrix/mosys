/*
 * Copyright 2003 Sun Microsystems, Inc.
 * Copyright 2010, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *    * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
 * log.h: based on logging code from ipmitool/lib/log.c
 */

#ifndef MOSYS_LIB_UTIL_LOG_H__
#define MOSYS_LIB_UTIL_LOG_H__

#include <stdio.h>

enum log_levels {
	LOG_EMERG = 0,		/* system is unusable */
	LOG_ALERT,		/* action must be taken immediately */
	LOG_CRIT,		/* critical conditions */
	LOG_ERR,		/* error conditions */
	LOG_WARNING,		/* warning conditions */
	LOG_NOTICE,		/* normal but significant condition */
	LOG_INFO,		/* informational */
	LOG_DEBUG,		/* debug-level messages */
};

/*
 * Init the logging subsystem.
 *
 * NOTE: name and output_file are *not* owned by the logging system, and
 * are not free()ed or close()d on log_halt().
 */
extern int mosys_log_init(const char *name, enum log_levels threshold,
                         FILE *output_file);
/* stop the logging subsystem */
extern int mosys_log_halt(void);

/* log something */
extern int lprintf(enum log_levels level, const char *format, ...);
/* log something including the errno string */
extern int lperror(enum log_levels level, const char *format, ...);

/* get the current log threshold */
extern int log_threshold_get(void);
/* set the current log threshold */
extern int log_threshold_set(enum log_levels threshold);

/* test if a log level is enabled */
extern int log_level_enabled(enum log_levels level);

#endif /* MOSYS_LIB_UTIL_LOG_H__ */
