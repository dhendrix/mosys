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

#ifndef MOSYS_LIB_ELOG_H_
#define MOSYS_LIB_ELOG_H_

#include <inttypes.h>
#include <sys/types.h>

enum smbios_log_entry_type;
struct platform_intf;
struct kv_pair;
struct smbios_log_entry;
struct smbios_table_log;

#define ELOG_HEADER_FORMAT	0x88
#define ELOG_MAGIC		0x474f4c45 /* 'ELOG' */
#define ELOG_VERSION		1

struct elog_header {
	uint32_t elog_magic;
	uint8_t elog_version;
	uint8_t elog_size;
	uint8_t reserved[2];
} __attribute__ ((packed));

extern int elog_print_type(struct platform_intf *intf,
                           struct smbios_log_entry *entry, struct kv_pair *kv);
extern int elog_print_data(struct platform_intf *intf,
                           struct smbios_log_entry *entry, struct kv_pair *kv);
extern int elog_verify(struct platform_intf *intf,
                       struct smbios_log_entry *entry);
extern int elog_verify_header(struct elog_header *elog_header);
extern int elog_print_multi(struct platform_intf *intf,
                            struct smbios_log_entry *entry, int start_id);
extern int elog_add_event_manually(struct platform_intf *intf,
				   enum smbios_log_entry_type type,
				   size_t data_size, uint8_t *data);
extern int elog_clear_manually(struct platform_intf *intf);
extern int elog_fetch_from_smbios(struct platform_intf *intf,
				  uint8_t **data, size_t *length,
				  off_t *header_offset, off_t *data_offset);
extern int elog_fetch_from_flash(struct platform_intf *intf,
				 uint8_t **data, size_t *length,
				 off_t *header_offset, off_t *data_offset);
extern int elog_write_to_flash(struct platform_intf *intf, uint8_t *data,
			       size_t length);

/*
 * Generic event log payloads modified by Google
 */
struct elog_clear_event {
	uint16_t bytes;
} __attribute__((packed));

/*
 * Event log events
 *
 * Note that for events that we write to elog from gsys, we also define a
 * _no_checksum type to send through the SMI interface.
 */

#define ELOG_TYPE_OS_EVENT                0x81
struct elog_os_event {
	uint8_t reason;
	uint8_t checksum;
} __attribute__ ((packed));
# define ELOG_OS_EVENT_CLEAN              0  /* Clean Shutdown */
# define ELOG_OS_EVENT_NMIWDT             1  /* NMI Watchdog */
# define ELOG_OS_EVENT_PANIC              2  /* Panic */
# define ELOG_OS_EVENT_OOPS               3  /* Oops */
# define ELOG_OS_EVENT_DIE                4  /* Die */
# define ELOG_OS_EVENT_MCE                5  /* MCE */
# define ELOG_OS_EVENT_SOFTWDT            6  /* Software Watchdog */
# define ELOG_OS_EVENT_MBE                7  /* MBE */
# define ELOG_OS_EVENT_TRIPLE             8  /* Triple Fault */
# define ELOG_OS_EVENT_THERMAL            9  /* Critical Thermal Threshold */

#define ELOG_TYPE_OS_BOOT                 0x90
struct elog_os_boot {
	uint32_t boot_count;
	uint8_t checksum;
} __attribute__ ((packed));

#define ELOG_TYPE_EC_EVENT                0x91
#define EC_EVENT_LID_CLOSED                0x01
#define EC_EVENT_LID_OPEN                  0x02
#define EC_EVENT_POWER_BUTTON              0x03
#define EC_EVENT_AC_CONNECTED              0x04
#define EC_EVENT_AC_DISCONNECTED           0x05
#define EC_EVENT_BATTERY_LOW               0x06
#define EC_EVENT_BATTERY_CRITICAL          0x07
#define EC_EVENT_BATTERY                   0x08
#define EC_EVENT_THERMAL_THRESHOLD         0x09
#define EC_EVENT_THERMAL_OVERLOAD          0x0a
#define EC_EVENT_THERMAL                   0x0b
#define EC_EVENT_USB_CHARGER               0x0c
#define EC_EVENT_KEY_PRESSED               0x0d
#define EC_EVENT_INTERFACE_READY           0x0e
#define EC_EVENT_KEYBOARD_RECOVERY         0x0f
#define EC_EVENT_THERMAL_SHUTDOWN          0x10
#define EC_EVENT_BATTERY_SHUTDOWN          0x11
struct elog_ec_event {
	uint8_t event;
	uint8_t checksum;
} __attribute__ ((packed));

/* Power */
#define ELOG_TYPE_POWER_FAIL              0x92
#define ELOG_TYPE_SUS_POWER_FAIL          0x93
#define ELOG_TYPE_PWROK_FAIL              0x94
#define ELOG_TYPE_SYS_PWROK_FAIL          0x95
#define ELOG_TYPE_POWER_ON                0x96
#define ELOG_TYPE_POWER_BUTTON            0x97
#define ELOG_TYPE_POWER_BUTTON_OVERRIDE   0x98

/* Reset */
#define ELOG_TYPE_RESET_BUTTON            0x99
#define ELOG_TYPE_SYSTEM_RESET            0x9a
#define ELOG_TYPE_RTC_RESET               0x9b
#define ELOG_TYPE_TCO_RESET               0x9c

/* Sleep/Wake */
#define ELOG_TYPE_ACPI_ENTER              0x9d
#define ELOG_TYPE_ACPI_WAKE               0x9e
#define ELOG_TYPE_WAKE_SOURCE             0x9f
#define ELOG_WAKE_SOURCE_PCIE              0x00
#define ELOG_WAKE_SOURCE_PME               0x01
#define ELOG_WAKE_SOURCE_PME_INTERNAL      0x02
#define ELOG_WAKE_SOURCE_RTC               0x03
#define ELOG_WAKE_SOURCE_GPIO              0x04
#define ELOG_WAKE_SOURCE_SMBUS             0x05
#define ELOG_WAKE_SOURCE_PWRBTN            0x06
struct elog_wake_source {
	uint8_t source;
	uint32_t instance;
	uint8_t checksum;
} __attribute__ ((packed));

/* Chrome OS related Events */
#define ELOG_TYPE_CROS_DEVELOPER_MODE    0xa0
#define ELOG_TYPE_CROS_RECOVERY_MODE     0xa1
struct elog_cros_recovery_mode {
	uint8_t reason;
	uint8_t checksum;
} __attribute__ ((packed));

/* Management Engine Events */
#define ELOG_TYPE_MANAGEMENT_ENGINE      0xa2
#define  ELOG_ME_PATH_NORMAL              0x00
#define  ELOG_ME_PATH_S3WAKE              0x01
#define  ELOG_ME_PATH_ERROR               0x02
#define  ELOG_ME_PATH_RECOVERY            0x03
#define  ELOG_ME_PATH_DISABLED            0x04
#define  ELOG_ME_PATH_FW_UPDATE           0x05

#define ELOG_TYPE_MANAGEMENT_ENGINE_EXT  0xa4
#define  ELOG_ME_PHASE_ROM                0
#define  ELOG_ME_PHASE_BRINGUP            1
#define  ELOG_ME_PHASE_UKERNEL            2
#define  ELOG_ME_PHASE_POLICY             3
#define  ELOG_ME_PHASE_MODULE             4
#define  ELOG_ME_PHASE_UNKNOWN            5
#define  ELOG_ME_PHASE_HOST               6
struct elog_event_data_me_extended {
	uint8_t current_working_state;
	uint8_t operation_state;
	uint8_t operation_mode;
	uint8_t error_code;
	uint8_t progress_code;
	uint8_t current_pmevent;
	uint8_t current_state;
} __attribute__ ((packed));

/* Last post code from previous boot */
#define ELOG_TYPE_LAST_POST_CODE         0xa3
#define ELOG_TYPE_POST_EXTRA             0xa6
#define  ELOG_TYPE_POST_EXTRA_PATH       0x01
#define   ELOG_DEV_PATH_TYPE_NONE          0
#define   ELOG_DEV_PATH_TYPE_ROOT          1
#define   ELOG_DEV_PATH_TYPE_PCI           2
#define   ELOG_DEV_PATH_TYPE_PNP           3
#define   ELOG_DEV_PATH_TYPE_I2C           4
#define   ELOG_DEV_PATH_TYPE_APIC          5
#define   ELOG_DEV_PATH_TYPE_DOMAIN        6
#define   ELOG_DEV_PATH_TYPE_CPU_CLUSTER   7
#define   ELOG_DEV_PATH_TYPE_CPU           8
#define   ELOG_DEV_PATH_TYPE_CPU_BUS       9
#define   ELOG_DEV_PATH_TYPE_IOAPIC        10

/* EC Shutdown Reason */
#define ELOG_TYPE_EC_SHUTDOWN            0xa5

/* ARM/generic versions of sleep/wake */
#define ELOG_TYPE_SLEEP                  0xa7
#define ELOG_TYPE_WAKE                   0xa8
#define ELOG_TYPE_FW_WAKE                0xa9


/* Recovery reason codes for EVENT_TYPE_CROS_RECOVERY_MODE */
/* Recovery not requested. */
#define VBNV_RECOVERY_NOT_REQUESTED   0x00
/* Recovery requested from legacy utility.  (Prior to the NV storage
 * spec, recovery mode was a single bitfield; this value is reserved
 * so that scripts which wrote 1 to the recovery field are
 * distinguishable from scripts whch use the recovery reasons listed
 * here. */
#define VBNV_RECOVERY_LEGACY          0x01
/* User manually requested recovery via recovery button */
#define VBNV_RECOVERY_RO_MANUAL       0x02
/* RW firmware failed signature check (neither RW firmware slot was valid) */
#define VBNV_RECOVERY_RO_INVALID_RW   0x03
/* S3 resume failed */
#define VBNV_RECOVERY_RO_S3_RESUME    0x04
/* TPM error in read-only firmware */
#define VBNV_RECOVERY_RO_TPM_ERROR    0x05
/* Shared data error in read-only firmware */
#define VBNV_RECOVERY_RO_SHARED_DATA  0x06
/* Test error from S3Resume() */
#define VBNV_RECOVERY_RO_TEST_S3      0x07
/* Test error from LoadFirmwareSetup() */
#define VBNV_RECOVERY_RO_TEST_LFS     0x08
/* Test error from LoadFirmware() */
#define VBNV_RECOVERY_RO_TEST_LF      0x09
/* RW firmware failed signature check (neither RW firmware slot was valid).
 * Recovery reason is VBNV_RECOVERY_RO_INVALID_RW_CHECK_MIN + the check value
 * for the slot which came closest to validating; see VBSD_LF_CHECK_* in
 * vboot_struct.h. */
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_NOT_DONE         0x10
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_DEV_MISMATCH     0x11
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_REC_MISMATCH     0x12
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_VERIFY_KEYBLOCK  0x13
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_KEY_ROLLBACK     0x14
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_DATA_KEY_PARSE   0x15
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_VERIFY_PREAMBLE  0x16
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_FW_ROLLBACK      0x17
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_HEADER_VALID     0x18
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_GET_FW_BODY      0x19
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_HASH_WRONG_SIZE  0x1A
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_VERIFY_BODY      0x1B
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_VALID            0x1C
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_NO_RO_NORMAL     0x1D
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_E                0x1E
#define VBNV_RECOVERY_RO_INVALID_RW_CHECK_F                0x1F
/* Firmware boot failure outside of verified boot (RAM init, missing SSD,
 * etc.). */
#define VBNV_RECOVERY_RO_FIRMWARE     0x20
/* Recovery mode TPM initialization requires a system reboot.  The system was
 * already in recovery mode for some other reason when this happened. */
#define VBNV_RECOVERY_RO_TPM_REBOOT   0x21
/* Other EC software sync error */
#define VBNV_RECOVERY_EC_SOFTWARE_SYNC 0x22
/* Unable to determine active EC image */
#define VBNV_RECOVERY_EC_UNKNOWN_IMAGE 0x23
/* EC software sync - error obtaining EC image hash (deprecated) */
#define VBNV_RECOVERY_DEP_EC_HASH         0x24
/* EC software sync - error obtaining expected EC image */
#define VBNV_RECOVERY_EC_EXPECTED_IMAGE 0x25
/* EC software sync - error updating EC */
#define VBNV_RECOVERY_EC_UPDATE       0x26
/* EC software sync - unable to jump to EC-RW */
#define VBNV_RECOVERY_EC_JUMP_RW      0x27
/* EC software sync - unable to protect / unprotect EC-RW */
#define VBNV_RECOVERY_EC_PROTECT      0x28
/* EC software sync - error obtaining expected EC hash */
#define VBNV_RECOVERY_EC_EXPECTED_HASH 0x29
/* EC software sync - expected EC image doesn't match hash */
#define VBNV_RECOVERY_EC_HASH_MISMATCH 0x2A
/* Unspecified/unknown error in read-only firmware */
#define VBNV_RECOVERY_RO_UNSPECIFIED  0x3F
/* User manually requested recovery by pressing a key at developer
 * warning screen */
#define VBNV_RECOVERY_RW_DEV_SCREEN   0x41
/* No OS kernel detected */
#define VBNV_RECOVERY_RW_NO_OS        0x42
/* OS kernel failed signature check */
#define VBNV_RECOVERY_RW_INVALID_OS   0x43
/* TPM error in rewritable firmware */
#define VBNV_RECOVERY_RW_TPM_ERROR    0x44
/* RW firmware in dev mode, but dev switch is off */
#define VBNV_RECOVERY_RW_DEV_MISMATCH 0x45
/* Shared data error in rewritable firmware */
#define VBNV_RECOVERY_RW_SHARED_DATA  0x46
/* Test error from LoadKernel() */
#define VBNV_RECOVERY_RW_TEST_LK      0x47
/* No bootable disk found */
#define VBNV_RECOVERY_DEP_RW_NO_DISK  0x48
/* Rebooting did not correct TPM_E_FAIL or TPM_E_FAILEDSELFTEST  */
#define VBNV_RECOVERY_TPM_E_FAIL      0x49
/* TPM setup error in read-only firmware */
#define VBNV_RECOVERY_RO_TPM_S_ERROR  0x50
/* TPM write error in read-only firmware */
#define VBNV_RECOVERY_RO_TPM_W_ERROR  0x51
/* TPM lock error in read-only firmware */
#define VBNV_RECOVERY_RO_TPM_L_ERROR  0x52
/* TPM update error in read-only firmware */
#define VBNV_RECOVERY_RO_TPM_U_ERROR  0x53
/* TPM read error in rewritable firmware */
#define VBNV_RECOVERY_RW_TPM_R_ERROR  0x54
/* TPM write error in rewritable firmware */
#define VBNV_RECOVERY_RW_TPM_W_ERROR  0x55
/* TPM lock error in rewritable firmware */
#define VBNV_RECOVERY_RW_TPM_L_ERROR  0x56
/* EC software sync unable to get EC image hash */
#define VBNV_RECOVERY_EC_HASH_FAILED  0x57
/* EC software sync invalid image hash size */
#define VBNV_RECOVERY_EC_HASH_SIZE    0x58
/* Unspecified error while trying to load kernel */
#define VBNV_RECOVERY_LK_UNSPECIFIED  0x59
/* No bootable storage device in system */
#define VBNV_RECOVERY_RW_NO_DISK      0x5A
/* No bootable kernel found on disk */
#define VBNV_RECOVERY_RW_NO_KERNEL    0x5B
/* Unspecified/unknown error in rewritable firmware */
#define VBNV_RECOVERY_RW_UNSPECIFIED  0x7F
/* DM-verity error */
#define VBNV_RECOVERY_KE_DM_VERITY    0x81
/* Unspecified/unknown error in kernel */
#define VBNV_RECOVERY_KE_UNSPECIFIED  0xBF
/* Recovery mode test from user-mode */
#define VBNV_RECOVERY_US_TEST         0xC1
/* Unspecified/unknown error in user-mode */
#define VBNV_RECOVERY_US_UNSPECIFIED  0xFF

#endif /* MOSYS_LIB_ELOG_H_ */
