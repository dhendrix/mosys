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
#include <stdlib.h>
#include <inttypes.h>
#include <valstr.h>

#include "lib/elog.h"
#include "lib/smbios.h"
#include "lib/string.h"

#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

/*
 * elog_verify_header - verify and validate if header is a valid Google Event
 *                      Log header.
 *
 * @header:  pointer to elog header to verify
 *
 * return < 0 if invalid, 0 if valid
 */
static int elog_verify_header(void *header)
{
	struct elog_header *hdr;

	if (header == NULL)
		return -1;

	hdr = header;

	if (hdr->elog_magic != ELOG_MAGIC)
		return -1;
	if (hdr->elog_size != sizeof(*hdr))
		return -1;
	if (hdr->elog_version != ELOG_VERSION)
		return -1;

	return 0;
}

int elog_verify_metadata(struct smbios_table_log *table, void *eventlog_header)
{
	MOSYS_DCHECK(table);

	if (table->header_format != ELOG_HEADER_FORMAT)
		return -1;
	if (elog_verify_header(eventlog_header) < 0)
		return -1;

	return 0;
}

/*
 * elog_print_type - add the type of the entry to the kv_pair
 *
 * @intf:   platform interface used for low level hardware access
 * @entry:  the smbios log entry to get type information
 * @kv:     kv_pair structure to add type information to
 *
 * Returns 0 on failure, 1 on success.
 */
int elog_print_type(struct platform_intf *intf, struct smbios_log_entry *entry,
                    struct kv_pair *kv)
{
	const char *type;
	const struct valstr elog_event_types[] = {
		{ ELOG_TYPE_OS_EVENT, "Kernel Event" },
		{ ELOG_TYPE_OS_BOOT, "OS Boot" },
		{ ELOG_TYPE_EC_EVENT, "EC Event" },
		{ ELOG_TYPE_POWER_FAIL, "Power Fail" },
		{ ELOG_TYPE_SUS_POWER_FAIL, "SUS Power Fail" },
		{ ELOG_TYPE_PWROK_FAIL, "PWROK Fail" },
		{ ELOG_TYPE_SYS_PWROK_FAIL, "SYS PWROK Fail" },
		{ ELOG_TYPE_POWER_ON, "Power On" },
		{ ELOG_TYPE_POWER_BUTTON, "Power Button" },
		{ ELOG_TYPE_POWER_BUTTON_OVERRIDE, "Power Button Override" },
		{ ELOG_TYPE_RESET_BUTTON, "Reset Button" },
		{ ELOG_TYPE_SYSTEM_RESET, "System Reset" },
		{ ELOG_TYPE_TCO_RESET, "RTC Reset" },
		{ ELOG_TYPE_TCO_RESET, "TCO Reset" },
		{ ELOG_TYPE_ACPI_ENTER, "ACPI Enter" },
		{ ELOG_TYPE_ACPI_WAKE, "ACPI Wake" },
		{ ELOG_TYPE_WAKE_SOURCE, "Wake Source" },
		{ ELOG_TYPE_CROS_DEVELOPER_MODE, "Chrome OS Developer Mode" },
		{ ELOG_TYPE_CROS_RECOVERY_MODE, "Chrome OS Recovery Mode" },
		{ ELOG_TYPE_MANAGEMENT_ENGINE, "Management Engine" },
		{ 0x0, NULL },
	};

	type = smbios_get_event_type_string(entry);

	if (type == NULL) {
		type = val2str_default(entry->type, elog_event_types, NULL);
	}

	if (type != NULL) {
		kv_pair_add(kv, "type", type);
		return 1;
	}

	lprintf(LOG_DEBUG, "elog: event type 0x%x unknown.\n", entry->type);

	return 0;
}

/*
 * elog_print_data - add the data associated with the entry to the kv_pair
 *
 * @intf:   platform interface used for low level hardware access
 * @entry:  the smbios log entry to get the data information
 * @kv:     kv_pair structure to add data to
 *
 * Returns 0 on failure, 1 on success.
 */
int elog_print_data(struct platform_intf *intf, struct smbios_log_entry *entry,
                    struct kv_pair *kv)
{
	static struct valstr os_events[] = {
		{ ELOG_OS_EVENT_CLEAN, "Clean Shutdown" },
		{ ELOG_OS_EVENT_NMIWDT, "NMI Watchdog" },
		{ ELOG_OS_EVENT_PANIC, "Panic" },
		{ ELOG_OS_EVENT_OOPS, "Oops" },
		{ ELOG_OS_EVENT_DIE, "Die" },
		{ ELOG_OS_EVENT_MCE, "MCE" },
		{ ELOG_OS_EVENT_SOFTWDT, "Software Watchdog" },
		{ ELOG_OS_EVENT_MBE, "Multi-bit Error" },
		{ ELOG_OS_EVENT_TRIPLE, "Triple Fault" },
		{ ELOG_OS_EVENT_THERMAL, "Critical Thermal Threshold" },
		{ 0, NULL },
	};
	static struct valstr wake_source_types[] = {
		{ ELOG_WAKE_SOURCE_PCIE, "PCI Express" },
		{ ELOG_WAKE_SOURCE_PME, "PCI PME" },
		{ ELOG_WAKE_SOURCE_PME_INTERNAL, "Internal PME" },
		{ ELOG_WAKE_SOURCE_RTC, "RTC Alarm" },
		{ ELOG_WAKE_SOURCE_GPIO, "GPIO" },
		{ ELOG_WAKE_SOURCE_SMBUS, "SMBALERT" },
		{ 0, NULL },
	};
	static struct valstr ec_event_types[] = {
		{ EC_EVENT_LID_CLOSED, "Lid Closed" },
		{ EC_EVENT_LID_OPEN, "Lid Open" },
		{ EC_EVENT_POWER_BUTTON, "Power Button" },
		{ EC_EVENT_AC_CONNECTED, "AC Connected" },
		{ EC_EVENT_AC_DISCONNECTED, "AC Disconnected" },
		{ EC_EVENT_BATTERY_LOW, "Battery Low" },
		{ EC_EVENT_BATTERY_CRITICAL, "Battery Critical" },
		{ EC_EVENT_BATTERY, "Battery" },
		{ EC_EVENT_THERMAL_THRESHOLD, "Thermal Threshold" },
		{ EC_EVENT_THERMAL_OVERLOAD, "Thermal Overload" },
		{ EC_EVENT_THERMAL, "Thermal" },
		{ EC_EVENT_USB_CHARGER, "USB Charger" },
		{ EC_EVENT_KEY_PRESSED, "Key Pressed" },
		{ EC_EVENT_INTERFACE_READY, "Host Interface Ready" },
		{ EC_EVENT_KEYBOARD_RECOVERY, "Keyboard Recovery" },
		{ EC_EVENT_THERMAL_SHUTDOWN,
		  "Thermal Shutdown in previous boot" },
		{ EC_EVENT_BATTERY_SHUTDOWN,
		  "Battery Shutdown in previous boot" },
		{ 0, NULL },
	};
	static struct valstr cros_recovery_reasons[] = {
		{ VBNV_RECOVERY_LEGACY, "Legacy Utility" },
		{ VBNV_RECOVERY_RO_MANUAL, "Recovery Button Pressed" },
		{ VBNV_RECOVERY_RO_INVALID_RW, "RW Failed Signature Check" },
		{ VBNV_RECOVERY_RO_S3_RESUME, "S3 Resume Failed" },
		{ VBNV_RECOVERY_RO_TPM_ERROR, "TPM Error in RO Firmware" },
		{ VBNV_RECOVERY_RO_SHARED_DATA,
		  "Shared Data Error in RO Firmware" },
		{ VBNV_RECOVERY_RO_TEST_S3, "Test Error from S3 Resume()" },
		{ VBNV_RECOVERY_RO_TEST_LFS,
		  "Test Error from LoadFirmwareSetup()" },
		{ VBNV_RECOVERY_RO_TEST_LF,
		  "Test Errofr from LoadFirmware()" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_0, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_1, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_2, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_3, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_4, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_5, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_6, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_7, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_8, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_9, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_A, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_B, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_C, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_D, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_E, "" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_F, "" },
		{ VBNV_RECOVERY_RO_FIRMWARE, "Firmware Boot Failure" },
		{ VBNV_RECOVERY_RO_TPM_REBOOT, "Recovery Mode TPM Reboot" },
		{ VBNV_RECOVERY_EC_SOFTWARE_SYNC,
		  "EC Software Sync Error" },
		{ VBNV_RECOVERY_EC_UNKNOWN_IMAGE,
		  "Unable to determine active EC image" },
		{ VBNV_RECOVERY_RO_UNSPECIFIED,
		  "Unknown Error in RO Firmware" },
		{ VBNV_RECOVERY_RW_DEV_SCREEN,
		  "User Requested from Developer Screen" },
		{ VBNV_RECOVERY_RW_NO_OS, "No OS Kernel Detected" },
		{ VBNV_RECOVERY_RW_INVALID_OS,
		  "OS Kernel Failed Signature Check" },
		{ VBNV_RECOVERY_RW_TPM_ERROR, "TPM Error in RW Firmware" },
		{ VBNV_RECOVERY_RW_DEV_MISMATCH,
		  "RW Dev Firmware but not Dev Mode" },
		{ VBNV_RECOVERY_RW_SHARED_DATA,
		  "Shared Data Error in RW Firmware" },
		{ VBNV_RECOVERY_RW_TEST_LK, "Test Error from LoadKernel()" },
		{ VBNV_RECOVERY_RW_NO_DISK, "No Bootable Disk Found" },
		{ VBNV_RECOVERY_RW_UNSPECIFIED,
		  "Unknown Error in RW Firmware" },
		{ VBNV_RECOVERY_KE_DM_VERITY, "DM-Verity Error" },
		{ VBNV_RECOVERY_KE_UNSPECIFIED, "Unknown Error in Kernel" },
		{ VBNV_RECOVERY_US_TEST, "Test from User Mode" },
		{ VBNV_RECOVERY_US_UNSPECIFIED, "Unknown Error in User Mode" },
		{ 0, NULL },
	};
	static struct valstr me_path_types[] = {
		{ ELOG_ME_PATH_NORMAL, "Normal" },
		{ ELOG_ME_PATH_NORMAL, "S3 Wake" },
		{ ELOG_ME_PATH_ERROR, "Error" },
		{ ELOG_ME_PATH_RECOVERY, "Recovery" },
		{ ELOG_ME_PATH_DISABLED, "Disabled" },
		{ ELOG_ME_PATH_FW_UPDATE, "Firmware Update" },
		{ 0, NULL },
	};

	switch (entry->type) {
	case SMBIOS_EVENT_TYPE_LOGCLEAR:
	{
		uint16_t *bytes = (void *)&entry->data[0];
		kv_pair_fmt(kv, "bytes", "%u", *bytes);
		break;
	}

	case SMBIOS_EVENT_TYPE_BOOT:
	{
		uint32_t *count = (void *)&entry->data[0];
		kv_pair_fmt(kv, "count", "%u", *count);
		break;
	}
	case ELOG_TYPE_OS_EVENT:
	{
		uint32_t *event = (void *)&entry->data[0];
		kv_pair_add(kv, "event", val2str(*event, os_events));
		break;
	}
	case ELOG_TYPE_ACPI_ENTER:
	case ELOG_TYPE_ACPI_WAKE:
	{
		uint8_t *state = (void *)&entry->data[0];
		kv_pair_fmt(kv, "state", "S%u", *state);
		break;
	}
	case ELOG_TYPE_WAKE_SOURCE:
	{
		struct elog_wake_source *event;
		event = (void *)&entry->data[0];
		kv_pair_add(kv, "source",
			    val2str(event->source, wake_source_types));
		kv_pair_fmt(kv, "instance", "%u", event->instance);
		break;
	}
	case ELOG_TYPE_EC_EVENT:
	{
		uint8_t *event = (void *)&entry->data[0];
		kv_pair_add(kv, "event", val2str(*event, ec_event_types));
		break;
	}
	case ELOG_TYPE_CROS_RECOVERY_MODE:
	{
		uint8_t *reason = (void *)&entry->data[0];
		kv_pair_add(kv, "reason",
			    val2str(*reason, cros_recovery_reasons));
		break;
	}
	case ELOG_TYPE_MANAGEMENT_ENGINE:
	{
		uint8_t *path = (void *)&entry->data[0];
		kv_pair_add(kv, "path", val2str(*path, me_path_types));
	}
	default:
		return 0;
	}

	return 0;
}

/*
 * elog_verify - verify the contents of the SMBIOS log entry.
 *
 * @intf:   platform interface used for low level hardware access
 * @entry:  the smbios log entry to verify
 *
 * returns 0 failure of verification, 1 if verification is sucessful
 */
int elog_verify(struct platform_intf *intf, struct smbios_log_entry *entry)
{
	char *data;
	char checksum;
	int i;

	checksum = 0;
	data = (void *)entry;

	for (i = 0; i < entry->length; i++) {
		checksum += data[i];
	}

	return checksum == 0;
}
