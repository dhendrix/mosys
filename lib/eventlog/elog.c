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
#include <fmap.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <valstr.h>

#include "intf/mmio.h"

#include "lib/coreboot.h"
#include "lib/eeprom.h"
#include "lib/elog.h"
#include "lib/elog_smbios.h"
#include "lib/math.h"
#include "lib/smbios.h"
#include "lib/string.h"

#include "mosys/alloc.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/output.h"
#include "mosys/platform.h"

/*
 * elog_verify_header - verify and validate if header is a valid Google Event
 *                      Log header.
 *
 * @header:  pointer to elog header to verify
 *
 * return < 0 if invalid, 0 if valid
 */
int elog_verify_header(struct elog_header *header)
{
	if (header == NULL)
		return -1;

	if (header->elog_magic != ELOG_MAGIC)
		return -1;
	if (header->elog_size != sizeof(*header))
		return -1;
	if (header->elog_version != ELOG_VERSION)
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
		{ ELOG_TYPE_RTC_RESET, "RTC Reset" },
		{ ELOG_TYPE_TCO_RESET, "TCO Reset" },
		{ ELOG_TYPE_ACPI_ENTER, "ACPI Enter" },
		{ ELOG_TYPE_ACPI_WAKE, "ACPI Wake" },
		{ ELOG_TYPE_WAKE_SOURCE, "Wake Source" },
		{ ELOG_TYPE_CROS_DEVELOPER_MODE, "Chrome OS Developer Mode" },
		{ ELOG_TYPE_CROS_RECOVERY_MODE, "Chrome OS Recovery Mode" },
		{ ELOG_TYPE_MANAGEMENT_ENGINE, "Management Engine" },
		{ ELOG_TYPE_MANAGEMENT_ENGINE_EXT, "Management Engine Extra" },
		{ ELOG_TYPE_LAST_POST_CODE, "Last post code in previous boot" },
		{ ELOG_TYPE_POST_EXTRA, "Extra info from previous boot" },
		{ ELOG_TYPE_EC_SHUTDOWN, "EC Shutdown" },
		{ ELOG_TYPE_SLEEP, "Sleep" },
		{ ELOG_TYPE_WAKE, "Wake" },
		{ ELOG_TYPE_FW_WAKE, "FW Wake" },
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

	/* Indicate unknown type in value pair */
	kv_pair_add(kv, "type", "Unknown");
	kv_pair_fmt(kv, "value", "0x%02x", entry->type);
	return 1;
}

/*
 * CMOS Extra log format:
 * [31:24] = Extra Log Type
 * [23:0]  = Extra Log Data
 *
 * If Extra Log Type is 0x01 then Data is Device Path
 * [23:16] = Device Type
 * [15:0]  = Encoded Device Path
 */
static int elog_print_post_extra(struct platform_intf *intf,
				 struct kv_pair *kv, uint32_t extra)
{
	const struct valstr path_type_values[] = {
		{ ELOG_DEV_PATH_TYPE_PCI, "PCI" },
		{ ELOG_DEV_PATH_TYPE_PNP, "PNP" },
		{ ELOG_DEV_PATH_TYPE_I2C, "I2C" },
		{ ELOG_DEV_PATH_TYPE_APIC, "APIC" },
		{ ELOG_DEV_PATH_TYPE_DOMAIN, "DOMAIN" },
		{ ELOG_DEV_PATH_TYPE_CPU_CLUSTER, "CPU Cluster" },
		{ ELOG_DEV_PATH_TYPE_CPU, "CPU" },
		{ ELOG_DEV_PATH_TYPE_CPU_BUS, "CPU Bus" },
		{ ELOG_DEV_PATH_TYPE_IOAPIC, "IO-APIC" },
		{ 0, NULL },
	};
	uint8_t type = (extra >> 16) & 0xff;

	/* Currently only know how to print device path */
	if ((extra >> 24) != ELOG_TYPE_POST_EXTRA_PATH) {
		kv_pair_fmt(kv, "extra", "0x%08x", extra);
		return 0;
	}

	kv_pair_add(kv, "device", val2str(type, path_type_values));

	/* Handle different device path types */
	switch (type) {
	case ELOG_DEV_PATH_TYPE_PCI:
		kv_pair_fmt(kv, "path", "%02x:%02x.%1x", (extra >> 8) & 0xff,
			    (extra >> 3) & 0x1f, (extra & 0x3));
		break;
	case ELOG_DEV_PATH_TYPE_PNP:
	case ELOG_DEV_PATH_TYPE_I2C:
		kv_pair_fmt(kv, "path", "%02x:%02x", (extra >> 8) & 0xff,
			    extra & 0xff);
		break;
	case ELOG_DEV_PATH_TYPE_APIC:
	case ELOG_DEV_PATH_TYPE_DOMAIN:
	case ELOG_DEV_PATH_TYPE_CPU_CLUSTER:
	case ELOG_DEV_PATH_TYPE_CPU:
	case ELOG_DEV_PATH_TYPE_CPU_BUS:
	case ELOG_DEV_PATH_TYPE_IOAPIC:
		kv_pair_fmt(kv, "path", "0x%04x", extra & 0xffff);
		break;
	}

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
		{ ELOG_WAKE_SOURCE_PWRBTN, "Power Button" },
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
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_NOT_DONE,
		  "RW firmware check not done" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_DEV_MISMATCH,
		  "RW firmware developer flag mismatch" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_REC_MISMATCH,
		  "RW firmware recovery flash mismatch" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_VERIFY_KEYBLOCK,
		  "RW firmware unable to verify keyblock" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_KEY_ROLLBACK,
		  "RW firmware key version rollback detected" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_DATA_KEY_PARSE,
		  "RW firmware unable to parse data key" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_VERIFY_PREAMBLE,
		  "RW firmware unable to verify preamble" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_FW_ROLLBACK,
		  "RW firmware version rollback detected" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_HEADER_VALID,
		  "RW firmware header is valid" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_GET_FW_BODY,
		  "RW firmware unable to get firmware body" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_HASH_WRONG_SIZE,
		  "RW firmware hash is wrong size" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_VERIFY_BODY,
		  "RW firmware unable to verify firmware body" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_VALID,
		  "RW firmware is valid" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_NO_RO_NORMAL,
		  "RW firmware read-only normal path is not supported" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_E,
		  "RW firmware invalid (14)" },
		{ VBNV_RECOVERY_RO_INVALID_RW_CHECK_F,
		  "RW firmware invalid (15)" },
		{ VBNV_RECOVERY_RO_FIRMWARE, "Firmware Boot Failure" },
		{ VBNV_RECOVERY_RO_TPM_REBOOT, "Recovery Mode TPM Reboot" },
		{ VBNV_RECOVERY_EC_SOFTWARE_SYNC,
		  "EC Software Sync Error" },
		{ VBNV_RECOVERY_EC_UNKNOWN_IMAGE,
		  "Unable to determine active EC image" },
		{ VBNV_RECOVERY_DEP_EC_HASH,
		  "EC software sync error obtaining EC image hash" },
		{ VBNV_RECOVERY_EC_EXPECTED_IMAGE,
		  "EC software sync error obtaining expected EC image from BIOS" },
		{ VBNV_RECOVERY_EC_UPDATE,
		  "EC software sync error updating EC" },
		{ VBNV_RECOVERY_EC_JUMP_RW,
		  "EC software sync unable to jump to EC-RW" },
		{ VBNV_RECOVERY_EC_PROTECT,
		  "EC software sync protection error" },
		{ VBNV_RECOVERY_EC_EXPECTED_HASH,
		  "EC software sync error obtaining expected EC hash from BIOS" },
		{ VBNV_RECOVERY_EC_HASH_MISMATCH,
		  "EC software sync error comparing expected EC hash and image" },
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
		{ VBNV_RECOVERY_DEP_RW_NO_DISK, "No Bootable Disk Found" },
		{ VBNV_RECOVERY_TPM_E_FAIL,
		  "TPM_E_FAIL or TPM_E_FAILEDSELFTEST" },
		{ VBNV_RECOVERY_RO_TPM_S_ERROR,
		  "TPM setup error in read-only firmware" },
		{ VBNV_RECOVERY_RO_TPM_W_ERROR,
		  "TPM write error in read-only firmware" },
		{ VBNV_RECOVERY_RO_TPM_L_ERROR,
		  "TPM lock error in read-only firmware" },
		{ VBNV_RECOVERY_RO_TPM_U_ERROR,
		  "TPM update error in read-only firmware" },
		{ VBNV_RECOVERY_RW_TPM_R_ERROR,
		  "TPM read error in rewritable firmware" },
		{ VBNV_RECOVERY_RW_TPM_W_ERROR,
		  "TPM write error in rewritable firmware" },
		{ VBNV_RECOVERY_RW_TPM_L_ERROR,
		  "TPM lock error in rewritable firmware" },
		{ VBNV_RECOVERY_EC_HASH_FAILED,
		  "EC software sync unable to get EC image hash" },
		{ VBNV_RECOVERY_EC_HASH_SIZE,
		  "EC software sync invalid image hash size" },
		{ VBNV_RECOVERY_LK_UNSPECIFIED,
		  "Unspecified error while trying to load kernel" },
		{ VBNV_RECOVERY_RW_NO_DISK,
		  "No bootable storage device in system" },
		{ VBNV_RECOVERY_RW_NO_KERNEL,
		  "No bootable kernel found on disk" },
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
	static struct valstr coreboot_post_codes[] = {
		{ POST_RESET_VECTOR_CORRECT, "Reset Vector Correct" },
		{ POST_ENTER_PROTECTED_MODE, "Enter Protected Mode" },
		{ POST_PREPARE_RAMSTAGE, "Prepare RAM stage" },
		{ POST_ENTRY_C_START, "RAM stage Start" },
		{ POST_PRE_HARDWAREMAIN, "Before Hardware Main" },
		{ POST_ENTRY_RAMSTAGE, "RAM stage Main" },
		{ POST_CONSOLE_READY, "Console is ready" },
		{ POST_CONSOLE_BOOT_MSG, "Console Boot Message" },
		{ POST_ENABLING_CACHE, "Before Enabling Cache" },
		{ POST_ENTER_ELF_BOOT, "Before ELF Boot" },
		{ POST_JUMPING_TO_PAYLOAD, "Before Jump to Payload" },
		{ POST_DEAD_CODE, "Dead Code" },
		{ POST_RESUME_FAILURE, "Resume Failure" },
		{ POST_OS_RESUME, "Before OS Resume" },
		{ POST_OS_BOOT, "Before OS Boot" },
		{ POST_DIE, "Coreboot Dead" },
		{ POST_BS_PRE_DEVICE, "Before Device Probe" },
		{ POST_BS_DEV_INIT_CHIPS, "Initialize Chips" },
		{ POST_BS_DEV_ENUMERATE, "Device Enumerate" },
		{ POST_BS_DEV_RESOURCES, "Device Resource Allocation" },
		{ POST_BS_DEV_ENABLE, "Device Enable" },
		{ POST_BS_DEV_INIT, "Device Initialize" },
		{ POST_BS_POST_DEVICE, "After Device Probe" },
		{ POST_BS_OS_RESUME_CHECK, "OS Resume Check" },
		{ POST_BS_OS_RESUME, "OS Resume" },
		{ POST_BS_WRITE_TABLES, "Write Tables" },
		{ POST_BS_PAYLOAD_LOAD, "Load Payload" },
		{ POST_BS_PAYLOAD_BOOT, "Boot Payload" },
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
	case ELOG_TYPE_LAST_POST_CODE:
	{
		uint16_t *code = (void *)&entry->data[0];
		kv_pair_fmt(kv, "code", "0x%02x", *code);
		kv_pair_add(kv, "desc",  val2str(*code, coreboot_post_codes));
		break;
	}
	case ELOG_TYPE_POST_EXTRA:
	{
		uint32_t *extra = (void *)&entry->data[0];
		elog_print_post_extra(intf, kv, *extra);
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

static int elog_print_entry_me_ext(struct platform_intf *intf,
				   struct smbios_log_entry *entry, int id,
				   const char *desc, const char *value)
{
	struct kv_pair *kv;

	if (!desc || !value)
		return 0;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "entry", "%d", id);
	smbios_eventlog_print_timestamp(intf, entry, kv);
	kv_pair_add(kv, "type", desc);
	kv_pair_add(kv, "value", value);
	kv_pair_print(kv);
	kv_pair_free(kv);
	return 1;
}

/*
 * elog_print_multi_me_ext  -  print management engine extended events
 *
 * @intf:	platform interface
 * @entry:	log entry
 * @start_id:	starting entry id
 *
 * returns 0 to indicate nothing was printed
 * returns >0 to indicate how many events were printed
 * returns <0 to indicate error
 */
static int elog_print_multi_me_ext(struct platform_intf *intf,
				   struct smbios_log_entry *entry,
				   int start_id)
{
	int num_msg = 0;
	const struct valstr *me_state_values;
	const struct elog_event_data_me_extended *me = (void *)&entry->data[0];
	const struct valstr me_cws_values[] = {
		{ 0x00, "Reset" },
		{ 0x01, "Initializing" },
		{ 0x02, "Recovery" },
		{ 0x05, "Normal" },
		{ 0x06, "Platform Disable Wait" },
		{ 0x07, "OP State Transition" },
		{ 0x08, "Invalid CPU Plugged In" },
		{ 0xFF, NULL }
	};
	const struct valstr me_opstate_values[] = {
		{ 0x00, "Preboot" },
		{ 0x01, "M0 with UMA" },
		{ 0x04, "M3 without UMA" },
		{ 0x05, "M0 without UMA" },
		{ 0x06, "Bring up" },
		{ 0x07, "M0 without UMA but with error" },
		{ 0xFF, NULL }
	};
	const struct valstr me_opmode_values[] = {
		{ 0x02, "Debug" },
		{ 0x03, "Soft Temporary Disable" },
		{ 0x04, "Security Override via Jumper" },
		{ 0x05, "Security Override via MEI Message" },
		{ 0xFF, NULL }
	};
	const struct valstr me_error_values[] = {
		{ 0x01, "Uncategorized Failure" },
		{ 0x03, "Image Failure" },
		{ 0x04, "Debug Failure" },
		{ 0xFF, NULL }
	};
	const struct valstr me_progress_values[] = {
		{ 0x00, "ROM Phase" },
		{ 0x01, "BUP Phase" },
		{ 0x02, "uKernel Phase" },
		{ 0x03, "Policy Module" },
		{ 0x04, "Module Loading" },
		{ 0x05, "Unknown" },
		{ 0x06, "Host Communication" },
		{ 0xFF, NULL }
	};
	const struct valstr me_pmevent_values[] = {
		{ 0x00, "Clean Moff->Mx wake" },
		{ 0x01, "Moff->Mx wake after an error" },
		{ 0x02, "Clean global reset" },
		{ 0x03, "Global reset after an error" },
		{ 0x04, "Clean Intel ME reset" },
		{ 0x05, "Intel ME reset due to exception" },
		{ 0x06, "Pseudo-global reset" },
		{ 0x07, "S0/M0->Sx/M3" },
		{ 0x08, "Sx/M3->S0/M0" },
		{ 0x09, "Non-power cycle reset" },
		{ 0x0a, "Power cycle reset through M3" },
		{ 0x0b, "Power cycle reset through Moff" },
		{ 0x0c, "Sx/Mx->Sx/Moff" },
		{ 0xFF, NULL }
	};
	const struct valstr me_progress_rom_values[] = {
		{ 0x00, "BEGIN" },
		{ 0x06, "DISABLE" },
		{ 0xFF, NULL }
	};
	const struct valstr me_progress_bup_values[] = {
		{ 0x00, "Initialization starts" },
		{ 0x01, "Disable the host wake event" },
		{ 0x04, "Flow determination start process" },
		{ 0x08, "Error reading/matching VSCC table in the descriptor" },
		{ 0x0a, "Check to see if straps say ME DISABLED" },
		{ 0x0b, "Timeout waiting for PWROK" },
		{ 0x0d, "Possibly handle BUP manufacturing override strap" },
		{ 0x11, "Bringup in M3" },
		{ 0x12, "Bringup in M0" },
		{ 0x13, "Flow detection error" },
		{ 0x15, "M3 clock switching error" },
		{ 0x18, "M3 kernel load" },
		{ 0x1c, "T34 missing - cannot program ICC" },
		{ 0x1f, "Waiting for DID BIOS message" },
		{ 0x20, "Waiting for DID BIOS message failure" },
		{ 0x21, "DID reported an error" },
		{ 0x22, "Enabling UMA" },
		{ 0x23, "Enabling UMA error" },
		{ 0x24, "Sending DID Ack to BIOS" },
		{ 0x25, "Sending DID Ack to BIOS error" },
		{ 0x26, "Switching clocks in M0" },
		{ 0x27, "Switching clocks in M0 error" },
		{ 0x28, "ME in temp disable" },
		{ 0x32, "M0 kernel load" },
		{ 0xFF, NULL }
	};
	const struct valstr me_progress_policy_values[] = {
		{ 0x00, "Entery into Policy Module" },
		{ 0x03, "Received S3 entry" },
		{ 0x04, "Received S4 entry" },
		{ 0x05, "Received S5 entry" },
		{ 0x06, "Received UPD entry" },
		{ 0x07, "Received PCR entry" },
		{ 0x08, "Received NPCR entry" },
		{ 0x09, "Received host wake" },
		{ 0x0a, "Received AC<>DC switch" },
		{ 0x0b, "Received DRAM Init Done" },
		{ 0x0c, "VSCC Data not found for flash device" },
		{ 0x0d, "VSCC Table is not valid" },
		{ 0x0e, "Flash Partition Boundary is outside address space" },
		{ 0x0f, "ME cannot access the chipset descriptor region" },
		{ 0x10, "Required VSCC values for flash parts do not match" },
		{ 0xFF, NULL }
	};
	const struct valstr me_progress_hostcomm_values[] = {
		{ 0x00, "Host Communication Established" },
		{ 0xFF, NULL }
	};

	/* Current Working State */
	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME Working State",
		val2str_default(me->current_working_state,
				me_cws_values, NULL));

	/* Current Operation State */
	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME Operation State",
		val2str_default(me->operation_state,
				me_opstate_values, NULL));

	/* Current Operation Mode */
	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME Operation Mode",
		val2str_default(me->operation_mode,
				me_opmode_values, NULL));

	/* Progress Phase */
	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME Progress Phase",
		val2str_default(me->progress_code,
				me_progress_values, NULL));

	/* Power Management Event */
	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME PM Event",
		val2str_default(me->current_pmevent,
				me_pmevent_values, NULL));

	/* Error Code (if non-zero) */
	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME Error Code",
		val2str_default(me->error_code,
				me_error_values, NULL));

	switch (me->progress_code) {
	case ELOG_ME_PHASE_ROM:
		me_state_values = me_progress_rom_values;
		break;
	case ELOG_ME_PHASE_BRINGUP:
		me_state_values = me_progress_bup_values;
		break;
	case ELOG_ME_PHASE_POLICY:
		me_state_values = me_progress_policy_values;
		break;
	case ELOG_ME_PHASE_HOST:
		me_state_values = me_progress_hostcomm_values;
		break;
	}

	num_msg += elog_print_entry_me_ext(
		intf, entry, start_id + num_msg, "ME Phase State",
		val2str_default(me->current_state,
				me_state_values, NULL));

	return num_msg;
}

/*
 * elog_print_multi  -  print multiple entries for an event
 *
 * @intf:	platform interface
 * @entry:	log entry
 * @start_id:	starting entry id
 *
 * returns 0 to indicate nothing was printed
 * returns >0 to indicate how many events were printed
 * returns <0 to indicate error
 */
int elog_print_multi(struct platform_intf *intf,
                     struct smbios_log_entry *entry, int start_id)
{
	switch (entry->type) {
	case ELOG_TYPE_MANAGEMENT_ENGINE_EXT:
		/* Management Engine Extended Event */
		return elog_print_multi_me_ext(intf, entry, start_id);
	}

	return 0;
}

/*
 * elog_update_checksum  -  update the checksum at the last byte
 *
 * @entry:	entry structure to update
 * @checksum:	what to set the checksum to
 */
static void elog_update_checksum(struct smbios_log_entry *entry,
				 uint8_t checksum)
{
	uint8_t *entry_data = (uint8_t *)entry;
	entry_data[entry->length - 1] = checksum;
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
	return rolling8_csum((void *)entry, entry->length) == 0;
}

/*
 * elog_fill_timestamp  -  populate timestamp in entry with current time
 *
 * @entry:	the entry to fill in
 *
 * returns 0 on succcess, non-zero on failure
 */
static int elog_fill_timestamp(struct smbios_log_entry *entry)
{
	time_t secs = time(NULL);
	struct tm tm;
	int res = 0;

	if (secs == -1)
		res = -1;

	if (gmtime_r(&secs, &tm) == NULL)
		res = -1;

	entry->second = bin2bcd(tm.tm_sec);
	entry->minute = bin2bcd(tm.tm_min);
	entry->hour   = bin2bcd(tm.tm_hour);
	entry->day    = bin2bcd(tm.tm_mday);
	entry->month  = bin2bcd(tm.tm_mon + 1);
	entry->year   = bin2bcd(tm.tm_year % 100);

	/* Basic sanity check whether rtc_get worked and of expected ranges */
	if (res || entry->month > 0x12 || entry->day > 0x31 ||
	    entry->hour > 0x23 || entry->minute > 0x59 ||
	    entry->second > 0x59) {
		entry->year   = 0;
		entry->month  = 0;
		entry->day    = 0;
		entry->hour   = 0;
		entry->minute = 0;
		entry->second = 0;
	}

	return res;
}

/*
 * elog_prepare_entry  -  fill out an event structure
 *
 * @buf:	buffer for the event
 * @entry_type	the type of the new entry
 * @data:	what data to append to the entry
 * @data_size:	how big the data is
 * returns 0 on success, non-zero on failure
 */
int elog_prepare_entry(void *buf, enum smbios_log_entry_type entry_type,
		       void *data, uint8_t data_size)
{
	struct smbios_log_entry *entry = buf;

	entry->type = entry_type;
	entry->length = sizeof(*entry) + data_size + 1;
	if (elog_fill_timestamp(entry))
		return -1;

	if (data_size)
		memcpy(&entry->data, data, data_size);

	/* Zero the checksum byte and then compute checksum */
	elog_update_checksum(entry, 0);
	elog_update_checksum(entry, -rolling8_csum((void *)entry,
						   entry->length));

	return 0;
}

struct elog_copy_events_params {
	uint8_t *dest;
	off_t to_skip;
	off_t skipped;
};

static int elog_copy_events(struct platform_intf *intf,
			    struct smbios_log_entry *entry,
			    void *arg, int *complete)
{
	struct elog_copy_events_params *params = arg;

	MOSYS_DCHECK(entry);
	MOSYS_DCHECK(arg);
	MOSYS_DCHECK(complete);

	if (entry->length == 0) {
		lprintf(LOG_WARNING, "Zero-length eventlog entry detected.\n");
		*complete = 1;
		return -1;
	}

	if (params->skipped < params->to_skip) {
		params->skipped += entry->length;
	} else {
		memcpy(params->dest, entry, entry->length);
		params->dest += entry->length;
	}

	return 0;
}

static int elog_events_size(struct platform_intf *intf,
			    struct smbios_log_entry *entry,
			    void *arg, int *complete)
{
	size_t *events_size = (size_t *)arg;

	if (entry->length == 0) {
		lprintf(LOG_WARNING, "Zero-length eventlog entry detected.\n");
		*complete = 1;
		return -1;
	}

	*events_size += entry->length;
	return 0;
}

/*
 * elog_add_event_manually - add an event by accessing the log directly.
 *
 * @intf:          platform interface used for low level hardware access
 * @type:          the type of event to add
 * @data_size:     the size of the data to add to the event
 * @data:          pointer to the data to add
 *
 * returns -1 on failure, 0 on success
 */
int elog_add_event_manually(struct platform_intf *intf,
			    enum smbios_log_entry_type type,
			    size_t event_data_size, uint8_t *event_data)
{
	size_t full_threshold, shrink_size;

	uint8_t *data;
	uint8_t *new_data = NULL;
	size_t length, data_size, events_size;
	size_t event_size = sizeof(struct smbios_log_entry) +
			    event_data_size + 1;
	off_t header_offset, data_offset;
	struct elog_copy_events_params params;

	if (!intf->cb->eventlog->fetch || !intf->cb->eventlog->write) {
		errno = ENOSYS;
		return -1;
	}

	if (intf->cb->eventlog->fetch(intf, &data, &length, &header_offset,
				      &data_offset))
		return -1;

	data_size = length - data_offset;

	/*
	 * Assume the full threshold is 3/4 the log size, and the shrink size
	 * is 1/4 the size.
	 */
	full_threshold = (length * 3) / 4;
	shrink_size = length / 4;

	if (event_size > shrink_size) {
		lprintf(LOG_WARNING, "Event size %d is too large.\n",
			event_size);
		return -1;
	}

	/* Figure out how much space the existing events take up. */
	events_size = 0;
	if (smbios_eventlog_foreach_event(intf, NULL, &elog_events_size,
					  &events_size)) {
		lprintf(LOG_ERR, "Eventlog is corrupt and must be cleared "
				"before adding new events.\n");
		return -1;
	}

	/* Shrink the log if it's going to exceed the full threshold. */
	if (events_size + event_size > full_threshold) {
		uint32_t skipped;

		new_data = mosys_malloc(length);
		memcpy(new_data, data, data_offset);
		memset(new_data + data_offset, 0xff, data_size);

		params.dest = new_data + data_offset;
		params.skipped = 0;
		params.to_skip = shrink_size;

		if (smbios_eventlog_foreach_event(intf, NULL, &elog_copy_events,
						  &params)) {
			free(new_data);
			return -1;
		}

		skipped = params.skipped;
		events_size -= skipped;
		data = new_data;
		elog_prepare_entry(data + data_offset + events_size,
				   SMBIOS_EVENT_TYPE_LOGCLEAR,
				   &skipped, sizeof(skipped));
		events_size += sizeof(struct smbios_log_entry) +
			sizeof(skipped) + 1;
	}

	/* Add the new event. */
	if (elog_prepare_entry(data + data_offset + events_size, type,
			       event_data, event_data_size)) {
		free(new_data);
		return -1;
	}

	if (intf->cb->eventlog->write(intf, data, length)) {
		free(new_data);
		return -1;
	}

	free(new_data);
	return 0;
}

/*
 * elog_clear_manually - clear the eventlog by reading and writing it directly.
 *
 * @intf:          platform interface used for low level hardware access
 *
 * returns -1 on failure, 0 on success
 */
int elog_clear_manually(struct platform_intf *intf)
{
	uint8_t *data;
	uint8_t *new_data;
	size_t length;
	off_t header_offset, data_offset, data_size;
	struct elog_copy_events_params params;
	uint32_t skipped;

	if (!intf->cb->eventlog->fetch || !intf->cb->eventlog->write) {
		errno = ENOSYS;
		return -1;
	}

	if (intf->cb->eventlog->fetch(intf, &data, &length, &header_offset,
				      &data_offset))
		return -1;

	data_size = length - data_offset;

	new_data = mosys_malloc(length);
	memcpy(new_data, data, data_offset);
	memset(new_data + data_offset, 0xff, data_size);

	params.dest = new_data + data_offset;
	params.skipped = 0;
	params.to_skip = data_size;
	if (smbios_eventlog_foreach_event(intf, NULL, &elog_copy_events,
					  &params)) {
		lprintf(LOG_WARNING, "Eventlog corrupt, proceeding...\n");
	}

	skipped = params.skipped;
	if (elog_prepare_entry(params.dest, SMBIOS_EVENT_TYPE_LOGCLEAR,
			       &skipped, sizeof(skipped))) {
		free(new_data);
		return -1;
	}

	if (intf->cb->eventlog->write(intf, new_data, length)) {
		free(new_data);
		return -1;
	}

	return 0;
}

/*
 * elog_fetch_from_smbios - fetch the eventlog from the SMBIOS tables.
 *
 * @intf:          platform interface used for low level hardware access
 * @data:          pointer to the fetched contents of the event log
 * @length:        pointer to the length of the event log
 * @header_offset: offset of the header in the event log
 * @data_offset:   offset of the first event in the event log
 *
 * returns -1 on failure, 0 on success
 */
int elog_fetch_from_smbios(struct platform_intf *intf, uint8_t **data,
			   size_t *length, off_t *header_offset,
			   off_t *data_offset)
{
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_LOG, 0, &table,
			      SMBIOS_LEGACY_ENTRY_BASE,
			      SMBIOS_LEGACY_ENTRY_LEN) < 0) {
		lprintf(LOG_WARNING, "Unable to find SMBIOS eventlog table.\n");
		return -1;
	}

	if (table.data.log.header_format != ELOG_HEADER_FORMAT)
		return -1;

	/* Only support memory mapped I/O access */
	if (table.data.log.method != SMBIOS_LOG_METHOD_TYPE_MEM)
		return -1;

	*length = table.data.log.length;
	*data = mosys_malloc(*length);
	*header_offset = table.data.log.header_start;
	*data_offset = table.data.log.data_start;

	if (mmio_read(intf, table.data.log.address.mem, *length, *data) < 0)
		return -1;

	return 0;
}

static int elog_find_log_in_flash(struct platform_intf *intf,
				  struct eeprom **eeprom_p,
				  struct eeprom_region **region_p)
{
	struct eeprom *eeprom;
	struct eeprom_region *region = NULL;
	int found = 0;

	for (eeprom = &intf->cb->eeprom->eeprom_list[0];
			eeprom->name; eeprom++) {

		if (!(eeprom->flags & EEPROM_FLAG_EVENTLOG))
			continue;

		for (region = &eeprom->regions[0]; region->flag; region++) {
			if (region->flag & EEPROM_FLAG_EVENTLOG) {
				found = 1;
				break;
			}
		}

		if (found)
			break;
	}

	if (!found) {
		lprintf(LOG_WARNING, "No ROM with eventlog regions found.\n");
		return -1;
	}

	/* TODO: for now we assume that the eventlog will be found using fmap */
	if (!(eeprom->flags & EEPROM_FLAG_FMAP) && region->name)
		return -1;

	*region_p = region;
	*eeprom_p = eeprom;
	return 0;
}

/*
 * elog_fetch_from_flash - fetch the eventlog from the flash.
 *
 * @intf:          platform interface used for low level hardware access
 * @data:          pointer to the fetched contents of the event log
 * @length:        pointer to the length of the event log
 * @header_offset: offset of the header in the event log
 * @data_offset:   offset of the first event in the event log
 *
 * returns -1 on failure, 0 on success
 */
int elog_fetch_from_flash(struct platform_intf *intf, uint8_t **data,
			  size_t *length, off_t *header_offset,
			  off_t *data_offset)
{
	struct eeprom *eeprom;
	struct eeprom_region *region;
	int bytes_read;

	if (elog_find_log_in_flash(intf, &eeprom, &region))
		return -1;

	bytes_read = eeprom->device->read_by_name(intf, eeprom,
						region->name, data);
	if (bytes_read < 0) {
		lprintf(LOG_WARNING, "Failed to read event log from flash.\n");
		return -1;
	}

	*length = bytes_read;
	*header_offset = 0;
	*data_offset = sizeof(struct elog_header);

	return 0;
}

/*
 * elog_write_to_flash - write the eventlog to flash.
 *
 * @intf:          platform interface used for low level hardware access
 * @data:          pointer to the contents of the event log
 * @length:        length of the event log
 *
 * returns -1 on failure, 0 on success
 */
int elog_write_to_flash(struct platform_intf *intf, uint8_t *data,
			size_t length)
{
	struct eeprom *eeprom;
	struct eeprom_region *region;
	int bytes_written;

	if (elog_find_log_in_flash(intf, &eeprom, &region))
		return -1;

	bytes_written = eeprom->device->write_by_name(intf, eeprom,
						      region->name, length,
						      data);
	if (bytes_written != length) {
		lprintf(LOG_WARNING, "Failed to write event log to flash.\n");
		return -1;
	}

	return 0;
}
