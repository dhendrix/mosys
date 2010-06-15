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
 * log.c: logging routines, based on logging code from ipmitool/lib/log.c
 */

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "mosys/log.h"

#define LOG_MSG_LENGTH	1024

struct logpriv {
	const char *name;
	enum log_levels threshold;
	FILE *outfile;
	int valid;
};
static struct logpriv logpriv;

/*
 * return -1 on fail
 * return 0 on success
 * return +1 if log-threshold was < level
 */
int lprintf(enum log_levels level, const char *format, ...)
{
	char logmsg[LOG_MSG_LENGTH];
	va_list vptr;

	if (!logpriv.valid) {
		return -1;
	}

	if (logpriv.threshold < level) {
		return 1;
	}
	va_start(vptr, format);
	vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
	va_end(vptr);

	fprintf(logpriv.outfile, "%s", logmsg);
	fflush(logpriv.outfile);

	return 0;
}

/*
 * return -1 on fail
 * return 0 on success
 * return +1 if log-threshold was < level
 */
int lperror(enum log_levels level, const char *format, ...)
{
	char logmsg[LOG_MSG_LENGTH];
	va_list vptr;

	if (!logpriv.valid) {
		return -1;
	}

	if (logpriv.threshold < level) {
		return 1;
	}

	va_start(vptr, format);
	vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
	va_end(vptr);

	fprintf(logpriv.outfile, "%s: %s\n", logmsg, strerror(errno));
	fflush(logpriv.outfile);

	return 0;
}

int mosys_log_init(const char *name, enum log_levels threshold,
                  FILE *output_file)
{
	if (logpriv.valid) {
		return -1;
	}

	if (output_file == NULL) {
		output_file = stderr;
	}

	logpriv.name = name;
	logpriv.threshold = threshold;
	logpriv.outfile = output_file;
	logpriv.valid = 1;

	return 0;
}

int mosys_log_halt(void)
{
	if (logpriv.valid) {
		logpriv.valid = 0;
		return 0;
	}
	return -1;
}

int log_threshold_get(void)
{
	if (!logpriv.valid) {
		return -1;
	}
	return logpriv.threshold;
}

int log_threshold_set(enum log_levels threshold)
{
	if (!logpriv.valid) {
		return -1;
	}

	logpriv.threshold = threshold;
	return 0;
}

int log_level_enabled(enum log_levels level)
{
	return (logpriv.valid && level >= logpriv.threshold);
}
