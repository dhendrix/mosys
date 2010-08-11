/*                                                                                                   
 * Copyright (C) 2010 Google Inc.                                                                    
 *                                                                                                   
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef MOSYS_LIB_SMBIOS_TABLES_H__
#define MOSYS_LIB_SMBIOS_TABLES_H__

#include <inttypes.h>

#define SMBIOS_LEGACY_ENTRY_BASE	0xf0000
#define SMBIOS_LEGACY_ENTRY_LEN		0x10000
#define SMBIOS_ENTRY_MAGIC		"_SM_"

/* Entry */
struct smbios_entry {
	uint8_t anchor_string[4];
	uint8_t entry_cksum;
	uint8_t entry_length;
	uint8_t major_ver;
	uint8_t minor_ver;
	uint16_t max_size;
	uint8_t entry_rev;
	uint8_t format_area[5];
	uint8_t inter_anchor_string[5];
	uint8_t inter_anchor_cksum;
	uint16_t table_length;
	uint32_t table_address;
	uint16_t table_entry_count;
	uint8_t bcd_revision;
} __attribute__ ((packed));

/* Header */
struct smbios_header {
	uint8_t type;
	uint8_t length;
	uint16_t handle;
} __attribute__ ((packed));

/* Type 0 - BIOS information */
struct smbios_table_bios {
	uint8_t vendor;
	uint8_t version;
	uint16_t start_address;
	uint8_t release_date;
	uint8_t rom_size_64k_blocks;
	uint32_t characteristics;
	uint8_t unused[4];	/* unused bytes */
	uint8_t extension[2];	/* v2.4+ */
	uint8_t major_ver;	/* v2.4+ */
	uint8_t minor_ver;	/* v2.4+ */
	uint8_t ec_major_ver;	/* v2.4+ */
	uint8_t ec_minor_ver;	/* v2.4+ */
} __attribute__ ((packed));

/* Type 1 - system information */
struct smbios_table_system {
	uint8_t manufacturer;
	uint8_t name;
	uint8_t version;
	uint8_t serial_number;
	uint8_t uuid[16];
	uint8_t wakeup_type;
	uint8_t sku_number;	/* v2.4+ */
	uint8_t family;		/* v2.4+ */
} __attribute__ ((packed));

/* Type 2 - baseboard information */
struct smbios_table_baseboard {
	uint8_t manufacturer;
	uint8_t name;
	uint8_t version;
	uint8_t serial_number;
	uint8_t asset_tag;
	uint8_t feature_flags;
	uint8_t chassis_location;
	uint16_t chassis_handle;
	uint8_t board_type;
	uint8_t num_cont_handles;
	uint16_t cont_handles[];
} __attribute__ ((packed));

/* Type 15 - event log */
struct smbios_table_log {
	uint16_t length;
	uint16_t header_start;
	uint16_t data_start;
	uint8_t method;
	uint8_t status;
	uint32_t change_token;
	union {
		struct {
			uint16_t index;
			uint16_t data;
		} io;
		uint32_t mem;
		uint16_t gpnv;
	} address;
	uint8_t header_format;
	uint8_t descriptor_num;
	uint8_t descriptor_len;
	uint8_t descriptor[];
} __attribute__ ((packed));

/* Event log Type 1 header */
struct smbios_log_header_type1 {
	uint8_t oem[5];		/* oem customization */
	uint8_t metw;		/* multiple event time window */
	uint8_t meci;		/* multiple event count increment */
	uint8_t reset_addr;	/* pre-boot event log reset CMOS address */
	uint8_t reset_index;	/* pre-boot event log reset CMOS bit index */
	uint8_t cksum_start;	/* CMOS checksum starting offset */
	uint8_t cksum_count;	/* CMOS checksum byte count */
	uint8_t cksum_offset;	/* CMOS checksum offset */
	uint8_t reserved[3];	/* future expansion */
	uint8_t revision;	/* header revision (=1) */
} __attribute__ ((packed));

/* Event log entry */
struct smbios_log_entry {
	uint8_t type;		/* entry type */
	uint8_t length;		/* data length */
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t data[0];	/* variable length data */
} __attribute__ ((packed));

/* the possible values for smbios_log_entry.type */
enum smbios_log_entry_type {
	SMBIOS_EVENT_TYPE_RESERVED = 0x00,	/* Reserved */
	SMBIOS_EVENT_TYPE_SBE = 0x01,		/* single-bit ECC error */
	SMBIOS_EVENT_TYPE_MBE = 0x02,		/* multi-bit ECC error */
	SMBIOS_EVENT_TYPE_PARITY = 0x03,	/* memory parity error */
	SMBIOS_EVENT_TYPE_BUSTMOUT = 0x04,	/* bus timeout */
	SMBIOS_EVENT_TYPE_IOCHECK = 0x05,	/* IO channel check */
	SMBIOS_EVENT_TYPE_SOFTNMI = 0x06,	/* software NMI */
	SMBIOS_EVENT_TYPE_POSTMEM = 0x07,	/* POST memory resize */
	SMBIOS_EVENT_TYPE_POSTERR = 0x08,	/* POST error */
	SMBIOS_EVENT_TYPE_PCIPERR = 0x09,	/* PCI parity error */
	SMBIOS_EVENT_TYPE_PCISERR = 0x0a,	/* PCI system error */
	SMBIOS_EVENT_TYPE_CPUFAIL = 0x0b,	/* CPU failure */
	SMBIOS_EVENT_TYPE_EISAFSTO = 0x0c,	/* EISA FailSafe Timer timeout */
	SMBIOS_EVENT_TYPE_CORRDIS = 0x0d,	/* correctable memory log disabled */
	SMBIOS_EVENT_TYPE_LOGDIS = 0x0e,	/* specific event type log disabled */
	SMBIOS_EVENT_TYPE_LIMIT = 0x10,		/* system limit exceeded (e.g.  temp) */
	SMBIOS_EVENT_TYPE_WDT = 0x11,		/* async HW timer (WDT) timeout */
	SMBIOS_EVENT_TYPE_SYSINFO = 0x12,	/* system config information */
	SMBIOS_EVENT_TYPE_HDINFO = 0x13,	/* hard disk information */
	SMBIOS_EVENT_TYPE_RECONFIG = 0x14,	/* system reconfigured */
	SMBIOS_EVENT_TYPE_UCCPUERR = 0x15,	/* uncorrectable CPU-complex error */
	SMBIOS_EVENT_TYPE_LOGCLEAR = 0x16,	/* log area  reset/cleared */
	SMBIOS_EVENT_TYPE_BOOT = 0x17,		/* system boot */
	SMBIOS_EVENT_TYPE_OEM = 0x80,		/* OEM-specific event */
	SMBIOS_EVENT_TYPE_ENDLOG = 0xff		/* end of log */
};

/* the possible values for log access method */
enum smbios_log_method_type {
	SMBIOS_LOG_METHOD_TYPE_IO8 = 0,		/* 8-bit indexed I/O */
	SMBIOS_LOG_METHOD_TYPE_IO8X2 = 1,	/* 2x8-bit indexed I/O */
	SMBIOS_LOG_METHOD_TYPE_IO16 = 2,	/* 16-bit indexed I/O */
	SMBIOS_LOG_METHOD_TYPE_MEM = 3,		/* 32-bit physical memory */
	SMBIOS_LOG_METHOD_TYPE_GPNV = 4,	/* general purpose non-volatile */
};

/* Type 16 - physical memory array */
struct smbios_table_memory_array {
	uint8_t location;			/* enumerated */
	uint8_t use;				/* enumerated */
	uint8_t error_correction;		/* enumerated */
	uint32_t capacity;			/* in KB */
	uint16_t memory_error_handle;		/* error information */
	uint16_t num_devices;			/* number of slots */
} __attribute__ ((packed));

/* memory array location */
enum smbios_memory_array_location {
	SMBIOS_MEMORY_ARRAY_LOC_OTHER = 0x01,
	SMBIOS_MEMORY_ARRAY_LOC_UNKNOWN = 0x02,
	SMBIOS_MEMORY_ARRAY_LOC_BOARD = 0x03,
	SMBIOS_MEMORY_ARRAY_LOC_ISA_CARD = 0x04,
	SMBIOS_MEMORY_ARRAY_LOC_EISA_CARD = 0x05,
	SMBIOS_MEMORY_ARRAY_LOC_PCI_CARD = 0x06,
	SMBIOS_MEMORY_ARRAY_LOC_MCA_CARD = 0x07,
	SMBIOS_MEMORY_ARRAY_LOC_PCMCIA_CARD = 0x08,
	SMBIOS_MEMORY_ARRAY_LOC_OTHER_CARD = 0x09,
	SMBIOS_MEMORY_ARRAY_LOC_NUBUS = 0x0a,
	SMBIOS_MEMORY_ARRAY_LOC_PC98_C20 = 0xa0,
	SMBIOS_MEMORY_ARRAY_LOC_PC98_C24 = 0xa1,
	SMBIOS_MEMORY_ARRAY_LOC_PC98_E = 0xa2,
	SMBIOS_MEMORY_ARRAY_LOC_PC98_LBUS = 0xa3,
};

/* memory array use */
enum smbios_memory_array_use {
	SMBIOS_MEMORY_ARRAY_USE_OTHER = 0x01,
	SMBIOS_MEMORY_ARRAY_USE_UNKNOWN = 0x02,
	SMBIOS_MEMORY_ARRAY_USE_SYSTEM = 0x03,
	SMBIOS_MEMORY_ARRAY_USE_VIDEO = 0x04,
	SMBIOS_MEMORY_ARRAY_USE_FLASH = 0x05,
	SMBIOS_MEMORY_ARRAY_USE_NVRAM = 0x06,
	SMBIOS_MEMORY_ARRAY_USE_CACHE = 0x07,
};

/* memory array error correction */
enum smbios_memory_array_ecc {
	SMBIOS_MEMORY_ARRAY_ECC_OTHER = 0x01,
	SMBIOS_MEMORY_ARRAY_ECC_UNKNOWN = 0x02,
	SMBIOS_MEMORY_ARRAY_ECC_NONE = 0x03,
	SMBIOS_MEMORY_ARRAY_ECC_PARITY = 0x04,
	SMBIOS_MEMORY_ARRAY_ECC_SBE = 0x05,
	SMBIOS_MEMORY_ARRAY_ECC_MBE = 0x06,
	SMBIOS_MEMORY_ARRAY_ECC_CRC = 0x07,
};

/* Type 17 - memory device */
struct smbios_table_memory_device {
	uint16_t memory_array_handle;
	uint16_t memory_error_handle;
	uint16_t total_width;			/* in bits, including ECC */
	uint16_t data_width;			/* in bits, no ECC */
	uint16_t size;				/* bit 15: 0=MB 1=KB */
	uint8_t form_factor;			/* enumerated, 9=DIMM */
	uint8_t device_set;			/* dimm group */
	uint8_t locator;			/* string */
	uint8_t bank_locator;			/* string */
	uint8_t type;				/* memory type enum */
	uint16_t type_detail;			/* memory type detail bitmask */
	uint16_t speed;				/* in MHz */
	uint8_t manufacturer;			/* string */
	uint8_t serial_number;			/* string */
	uint8_t asset_tag;			/* string */
	uint8_t part_number;			/* string */
} __attribute__ ((packed));

/* memory device types */
enum smbios_memory_type {
	SMBIOS_MEMORY_TYPE_OTHER = 0x01,
	SMBIOS_MEMORY_TYPE_UNKNOWN = 0x02,
	SMBIOS_MEMORY_TYPE_DRAM = 0x03,
	SMBIOS_MEMORY_TYPE_EDRAM = 0x04,
	SMBIOS_MEMORY_TYPE_VRAM = 0x05,
	SMBIOS_MEMORY_TYPE_SRAM = 0x06,
	SMBIOS_MEMORY_TYPE_RAM = 0x07,
	SMBIOS_MEMORY_TYPE_ROM = 0x08,
	SMBIOS_MEMORY_TYPE_FLASH = 0x09,
	SMBIOS_MEMORY_TYPE_EEPROM = 0x0a,
	SMBIOS_MEMORY_TYPE_FEPROM = 0x0b,
	SMBIOS_MEMORY_TYPE_EPROM = 0x0c,
	SMBIOS_MEMORY_TYPE_CDRAM = 0x0d,
	SMBIOS_MEMORY_TYPE_3DRAM = 0x0e,
	SMBIOS_MEMORY_TYPE_SDRAM = 0x0f,
	SMBIOS_MEMORY_TYPE_SGRAM = 0x10,
	SMBIOS_MEMORY_TYPE_RDRAM = 0x11,
	SMBIOS_MEMORY_TYPE_DDR = 0x12,
	SMBIOS_MEMORY_TYPE_DDR2 = 0x13,
	SMBIOS_MEMORY_TYPE_DDR2_FBDIMM = 0x14,
	SMBIOS_MEMORY_TYPE_DDR3 = 0x18,
};

/* memory device type details (bitmask) */
enum smbios_memory_type_detail {
	SMBIOS_MEMORY_TYPE_DETAIL_OTHER = 0x0001,
	SMBIOS_MEMORY_TYPE_DETAIL_UNKNOWN = 0x0002,
	SMBIOS_MEMORY_TYPE_DETAIL_FAST_PAGED = 0x0004,
	SMBIOS_MEMORY_TYPE_DETAIL_STATIC_COL = 0x0008,
	SMBIOS_MEMORY_TYPE_DETAIL_PSEUDO_STATIC = 0x0010,
	SMBIOS_MEMORY_TYPE_DETAIL_RAMBUS = 0x0020,
	SMBIOS_MEMORY_TYPE_DETAIL_SYNCHRONOUS = 0x0040,
	SMBIOS_MEMORY_TYPE_DETAIL_CMOS = 0x0080,
	SMBIOS_MEMORY_TYPE_DETAIL_EDO = 0x0100,
	SMBIOS_MEMORY_TYPE_DETAIL_WINDOW_DRAM = 0x0200,
	SMBIOS_MEMORY_TYPE_DETAIL_CACHE_DRAM = 0x0400,
	SMBIOS_MEMORY_TYPE_DETAIL_NON_VOLATILE = 0x0800,
};

/* Type 32 - System Boot Information */
struct smbios_table_system_boot {
	uint8_t reserved[6];			/* Should be all zeros */
	uint8_t boot_status[0];			/* Variable boot status */
} __attribute__ ((packed));

/* The system boot number may be exposed in the type 32 table. It is identified
 * by the Vendor/OEM identifier 128. This is Google specific. */
#define BOOT_STATUS_BOOT_NUMBER 128
struct smbios_system_boot_number {
	uint8_t identifier;
	uint32_t boot_number;
} __attribute__((packed));

/* The length and number of strings defined here is not a limitation of SMBIOS.
 * These numbers were deemed good enough during development. */
#define SMBIOS_MAX_STRINGS 10
#define SMBIOS_MAX_STRING_LENGTH 64
/* One structure to rule them all */
struct smbios_table {
	struct smbios_header header;
	union {
		struct smbios_table_bios bios;
		struct smbios_table_system system;
		struct smbios_table_baseboard baseboard;
		struct smbios_table_log log;
		struct smbios_table_memory_array mem_array;
		struct smbios_table_memory_device mem_device;
		struct smbios_table_system_boot system_boot;
		uint8_t data[1024];
	} data;
	char string[SMBIOS_MAX_STRINGS][SMBIOS_MAX_STRING_LENGTH];
};

#endif /* MOSYS_LIB_SMBIOS_TABLES_H__ */
