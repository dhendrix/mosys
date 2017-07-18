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

#include <inttypes.h>
#include <unistd.h>

#include "stout.h"
#include "mosys/log.h"
#include "mosys/platform.h"

/* returns 0 to indicate success, <0 to indicate failure */
static int stout_wait_battery_auth_complete(struct platform_intf *intf)
{
	uint8_t status;
	do {
		if (ec_command(intf, STOUT_ECCMD_GET_BATTERY_AUTH_STATUS,
			NULL, 0, &status, 1) < 0)
			return -1;

		if (status == 0xff)
			return -1;
		else if (status == 0xaa)
			break;

		sleep(2);
	} while (1);

	return 0;
}

/*
 * stout_get_battery_fud - return battery first use date
 *
 * @intf:	platform interface
 *
 * returns pointer to first use date string if successful.
 * returns NULL to indicate failure.
 */
static const char *stout_get_battery_fud(struct platform_intf *intf)
{
	static char first_use_date[11];
	uint8_t status, date_hi, date_lo;
	int day, month, year;

	if (ecram_read(intf, STOUT_ECMEM_BATTERY_STATUS, &status,
	               STOUT_ECCMD_MEM_READ) < 0)
		return NULL;

	/* Bit 7: main battery attached. */
	if ((status & 0x80) == 0) {
		lprintf(LOG_DEBUG, "%s: no battery present.\n", __func__);
		return NULL;
	}

	if (stout_wait_battery_auth_complete(intf) < 0) {
		lprintf(LOG_DEBUG, "%s: Battery auth wait failed.\n", __func__);
		return NULL;
	}

	if (ec_command(intf, STOUT_ECCMD_GET_BATTERY_FIRST_USE_DATE_HI, NULL,
		0, &date_hi, 1) < 0 )
		return NULL;

	if (ec_command(intf, STOUT_ECCMD_GET_BATTERY_FIRST_USE_DATE_LO, NULL,
		0, &date_lo, 1) < 0 )
		return NULL;

	year = ((date_hi & 0xFE) >> 1) + 1980;
	month = (date_hi & 0x01) * 8 + ((date_lo & 0xE0) >> 5);
	day = (date_lo & 0x1F);

	sprintf(first_use_date, "%04d/%02d/%02d", year, month, day);
	return first_use_date;
}

/*
 * stout_set_battery_fud - Sets battery first use date.
 *
 * @intf:	platform interface
 * day: 	day of month to set
 * month:	month of year to set
 * year:	year to set
 *
 * returns 0 if successful or < 0 if failure.
 */
static int stout_set_battery_fud(struct platform_intf *intf,
				int day, int month, int year)
{
	uint8_t status, date_hi, date_lo;

	if (day < 0 || day > 31 || month < 0 || month > 12 || year < 1980) {
		lprintf(LOG_DEBUG, "%s: invalid date setting - %d %d %d.\n",
			__func__, day, month, year);
		return -1;
	}

	if (ecram_read(intf, STOUT_ECMEM_BATTERY_STATUS, &status,
	               STOUT_ECCMD_MEM_READ) < 0)
		return -1;

	/* Bit 7: main battery attached. */
	if ((status & 0x80) == 0) {
		lprintf(LOG_DEBUG, "%s: no battery present.\n", __func__);
		return -1;
	}

	if (stout_wait_battery_auth_complete(intf) < 0) {
		lprintf(LOG_DEBUG, "%s: Battery auth wait failed.\n", __func__);
		return -1;
	}

	/* Make sure FUD has not already been set. */
	if (ec_command(intf, STOUT_ECCMD_GET_BATTERY_FIRST_USE_DATE_STATUS,
		NULL, 0, &status, 1) < 0)
		return -1;

	if (status == 0) {
		lprintf(LOG_DEBUG, "%s: FUD already set.\n", __func__);
		return -1;
	}

	/* Attempt to set FUD */
	lprintf(LOG_DEBUG, "%s: Set FUD to %04d/%02d/%02d.\n", __func__,
		year, month, day);

	date_hi = ((year - 1980) << 1) + (month >> 3);
	date_lo = (month & 0x07) << 5;
	date_lo |= day;

	if (ecram_write(intf, STOUT_ECMEM_BATTERY_FIRST_USE_DATE_HI,
		date_hi) < 0)
		return -1;
	if (ecram_write(intf, STOUT_ECMEM_BATTERY_FIRST_USE_DATE_LO,
		date_lo) < 0)
		return -1;

	if (ec_command(intf, STOUT_ECCMD_LATCH_BATTERY_FIRST_USE_DATE,
		NULL, 0, &status, 1) < 0)
		return -1;

	return 0;
}

/*
 * stout_update_battery_fw - Attempts to update battery firmware.
 *
 * @intf:	platform interface
 *
 * returns 0 if successful or < 0 if failure.
 */
static int stout_update_battery_fw(struct platform_intf *intf)
{
	uint8_t status;
	stout_ec_command cmd = STOUT_ECCMD_MEM_READ;

	if (ecram_read(intf, STOUT_ECMEM_BATTERY_STATUS, &status, cmd) < 0)
		return -1;

	/* Bit 7: main battery attached. */
	if ((status & 0x80) == 0) {
		lprintf(LOG_DEBUG, "%s: no battery present.\n", __func__);
		return -1;
	}

	/* Check if update needed. */
	if (ec_command(intf, STOUT_ECCMD_BATTERY_FW_UPDATE_NEEDED,
		NULL, 0, &status, 1) < 0)
		return -1;

	if (status != 0) {
		lprintf(LOG_DEBUG, "%s: no battery fw update needed: %d.\n",
			__func__, status);
		return -1;
	}

	/* Start update. */
	if (ecram_read(intf, STOUT_ECMEM_BATTERY_FW_UPDATE, &status, cmd) < 0)
		return -1;

	lprintf(LOG_DEBUG, "%s: Starting FW update: %x.\n",
		__func__, status);

	if (ecram_write(intf, STOUT_ECMEM_BATTERY_FW_UPDATE, status | 0x20) < 0)
		return -1;

	/* Wait for update completion. */
	while (1) {
		if (ec_command(intf, STOUT_ECCMD_BATTERY_FW_UPDATE_STATUS,
			NULL, 0, &status, 1) < 0)
			return -1;

		if (status == 1)
			break;

		sleep(1);
	}

	lprintf(LOG_DEBUG, "%s: finished FW update: %x.\n",
		__func__, status);

	/* Clear update flag and check competion status. */
	if (ecram_read(intf, STOUT_ECMEM_BATTERY_FW_UPDATE, &status, cmd) < 0)
		return -1;

	if (ecram_write(intf, STOUT_ECMEM_BATTERY_FW_UPDATE,
		status & ~(0x20)) < 0)
		return -1;

	if (ec_command(intf, STOUT_ECCMD_BATTERY_FW_UPDATE_COMPLETION_STATUS,
		NULL, 0, &status, 1) < 0)
		return -1;

	if (status == 0) {
		lprintf(LOG_DEBUG, "%s: battery FW updated.\n",
			__func__);
		return 0;
	} else {
		lprintf(LOG_DEBUG, "%s: battery FW update failed: %d.\n",
			__func__, status);
		return -1;
	}
}

struct battery_cb stout_battery_cb = {
	.get_fud = stout_get_battery_fud,
	.set_fud = stout_set_battery_fud,
	.update  = stout_update_battery_fw,
};
