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

#ifndef MOSYS_PLATFORM_H__
#define MOSYS_PLATFORM_H__

#include <sys/types.h>

#include "lib/elog.h"

#include "mosys/mosys.h"

struct kv_pair;
struct gpio_map;
struct nonspd_mem_info;

#if 0
/* disabled via build option for mosys standalone binary */
#ifndef MOSYS_AS_LIBRARY
#define MOSYS_AS_LIBRARY 1
#endif
#endif

#define MOSYS_DATA_ROOT		"/var/log/mosysdata"

/* defined platform types */
enum platform_type {
	PLATFORM_DEFAULT,
	PLATFORM_X86,
	PLATFORM_X86_64,
	PLATFORM_ARMV7,
	PLATFORM_ARMV8,
};

/*
 *  Command types. These are exposed via "mosys -tv" to help with automated
 *  testing.
 */
enum arg_type {
	ARG_TYPE_GETTER,	/* "getter" functions */
	ARG_TYPE_SETTER,	/* "setter" functions */
	ARG_TYPE_SUB,		/* branch deeper into command hierachy */
};

/* nested command lists */
struct platform_intf;
struct platform_cmd {
	const char *name;		/* command name */
	const char *desc;		/* command help text */
	const char *usage;		/* command usage text */
	enum arg_type type;		/* argument type */
	union {				/* sub-commands or function */
		struct platform_cmd *sub;
		int (*func)(struct platform_intf *intf,
			    struct platform_cmd *cmd,
			    int argc, char **argv);
	} arg;
};

/* platform operations handlers */
struct pci_intf;
struct mmio_intf;
struct io_intf;
struct smi_intf;
struct i2c_intf;
struct amb_intf;
struct sp_intf;
struct msr_intf;
struct mce_intf;
struct edac_intf;
struct cpuid_intf;

struct platform_op {
	struct pci_intf *pci;		/* pci interface */
	struct mmio_intf *mmio;		/* mmio interface */
	struct io_intf *io;		/* io interface */
	struct smi_intf *smi;		/* smi interface */
	struct i2c_intf *i2c;		/* i2c interface */
	struct amb_intf *amb;		/* amb interface */
	struct sp_intf *sp;		/* sensorpath interface */
	struct msr_intf *msr;		/* msr interface */
	struct mce_intf *mce;		/* mcelog interface */
	struct edac_intf *edac;		/* edac interface */
	struct cpuid_intf *cpuid;	/* cpuid instruction interface */
	struct fru_intf *fru;		/* FRU interface */
	struct sched_intf *sched;	/* process scheduler interface */
};

/* sensor related callbacks */
struct sensor;
struct sensor_array;
struct sensor_cb {
	struct sensor_array *(*get_sensors)(struct platform_intf *intf);
	void (*add_sensors)(struct platform_intf *intf,
	                    struct sensor_array *array);

	/* methods for reading data and setting thresholds */
	int (*read_fantach)(struct platform_intf *intf, const char *name);
	int (*set_fantach)(struct platform_intf *intf, const char *sensor_name,
	                   unsigned int percent);
	int (*set_fantach_auto)(struct platform_intf *intf,
	                        const char *sensor_name);
	int (*set_fantach_off)(struct platform_intf *intf,
	                       const char *sensor_name);

	int (*read_thermal)(struct platform_intf *intf, const char *name);
	int (*set_thermal)(struct platform_intf *intf, const char *sensor_name,
	                   const char *param, double val);

	int (*read_voltage)(struct platform_intf *intf, const char *name);
	int (*set_voltage)(struct platform_intf *intf, const char *sensor_name,
	                   const char *param, double val);

	/* current and power are typically not set explicitly */
	int (*read_current)(struct platform_intf *intf, const char *name);
	int (*read_power)(struct platform_intf *intf, const char *name);
};

/*
 * mappings of logical DIMM to physical
 * not all map types apply to all platforms
 */
enum dimm_map_type {
	DIMM_TO_BUS,			/* SPD bus */
	DIMM_TO_ADDRESS,		/* SPD address */
	DIMM_TO_CHANNEL,		/* AMB channel */
	DIMM_TO_DEVICE,			/* AMB device */
	DIMM_TO_NODE,			/* NUMA node */
};

/* dimm error types */
enum dimm_error_type {
	DIMM_ERROR_SBE = 1,		/* single-bit error */
	DIMM_ERROR_MBE = 2,		/* multi-bit error */
};

/* dimm error threshold types */
enum dimm_threshold_type {
	DIMM_THRESHOLD_EVENTLOG = 1,	/* eventlog */
	DIMM_THRESHOLD_COUNT = 2,	/* counter */
};

#define DIMM_ALL_MASK	0xffff

/* memory related callbacks */
struct smbios_table;
struct memory_component;
struct memory_spd_cb {
	int (*read)(struct platform_intf *intf,
	            int dimm, int reg, int len, unsigned char *buf);
	int (*write)(struct platform_intf *intf,
	             int dimm, int reg, int len, unsigned char *buf);
};

struct memory_cb {
	int (*dimm_count)(struct platform_intf *intf);
	int (*dimm_map)(struct platform_intf *intf,
	                enum dimm_map_type type, int dimm);
	int (*dimm_convert)(struct platform_intf *intf,
	                    uint64_t address);
	int (*dimm_location)(struct platform_intf *intf, int array,
	                     const char *location);
	int (*dimm_print)(struct platform_intf *intf, int array, int index,
	                  struct smbios_table *device, struct kv_pair *kv);
	int (*dimm_test)(struct platform_intf *intf,
	                 const char *type, int dimm, int count);
	int (*dimm_speed)(struct platform_intf *intf,
	                  int dimm, struct kv_pair *kv);
	int (*dimm_present)(struct platform_intf *intf, int dimm);
	uint32_t (*error_count)(struct platform_intf *intf,
	                        enum dimm_error_type type, int dimm);
	int (*error_clear)(struct platform_intf *intf,
	                   enum dimm_error_type type, int dimm);
	int (*error_threshold)(struct platform_intf *intf,
	                       enum dimm_threshold_type type, uint32_t value);
	/* Translate an address to its components. Fill in the array of
	 * components of size num_components with components that make up the
	 * physical_address. num_components should be updated to reflect the
	 * number of entries added to the array. */
	int (*address_translate)(struct platform_intf *intf,
	                         uint64_t physical_address,
	                         struct memory_component *components,
	                         size_t *num_components);

	struct memory_spd_cb *spd;

	/* for systems where SPD-formatted memory info is not available */
	int (*nonspd_mem_info)(struct platform_intf *intf,
				const struct nonspd_mem_info **info);
};

/* eventlog related callbacks */
enum smbios_log_entry_type;
struct smbios_log_entry;
struct smbios_table_log;
struct eventlog_cb {
	int (*print_type)(struct platform_intf *intf,
	                  struct smbios_log_entry *entry,
	                  struct kv_pair *kv);
	int (*print_data)(struct platform_intf *intf,
	                  struct smbios_log_entry *entry,
	                  struct kv_pair *kv);
	int (*print_multi)(struct platform_intf *intf,
			   struct smbios_log_entry *entry,
			   int start_id);
	int (*verify)(struct platform_intf *intf,
	              struct smbios_log_entry *entry);
	int (*verify_header)(struct elog_header *elog_header);
	int (*add)(struct platform_intf *intf, enum smbios_log_entry_type type,
		   size_t data_size, uint8_t *data);
	int (*clear)(struct platform_intf *intf);
	int (*fetch)(struct platform_intf *intf, uint8_t **data,
		     size_t *length, off_t *header_offset, off_t *data_offset);
	int (*write)(struct platform_intf *intf, uint8_t *data, size_t length);
};

/* boot number callbacks */
struct bootnum_cb {
	uint32_t(*read)(struct platform_intf *intf);
	int (*reset)(struct platform_intf *intf);
};

/* NVRAM callbacks */
struct nvram_cb {
	int (*list)(struct platform_intf *intf);
	int (*clear)(struct platform_intf *intf);
	int (*dump)(struct platform_intf *intf);
	int (*vboot_read)(struct platform_intf *intf);
	int (*vboot_write)(struct platform_intf *intf,
			   const char *hexstring);
};

/* EEPROM and EEPROM-related callbacks */
struct eeprom_enet_cb {
	int (*read)(struct platform_intf *intf, int argc, char **argv);
	int (*write)(struct platform_intf *intf, int argc, char **argv);
};
struct eeprom_cb {
	struct eeprom_enet_cb *enet;
	struct eeprom *eeprom_list;
};

enum led_state {
	LED_STATE_OFF,
	LED_STATE_ON,
	LED_STATE_STANDBY,
	LED_STATE_SLOW,
	LED_STATE_FAST,
};

enum storage_phy_speed {
	PHY_SPEED_UNKNOWN,
	PHY_SPEED_SATA1,
	PHY_SPEED_SATA2,
	PHY_SPEED_SATA3,
};

/* smbios callbacks */
struct smbios_cb {
	char *(*bios_vendor)(struct platform_intf *intf);
	char *(*system_vendor)(struct platform_intf *intf);
	char *(*system_name)(struct platform_intf *intf);
	char *(*system_version)(struct platform_intf *intf);
	char *(*system_family)(struct platform_intf *intf);
	char *(*system_sku)(struct platform_intf *intf);
	char *(*system_serial)(struct platform_intf *intf);
};

/* vpd callbacks */
struct vpd_cb {
	char *(*system_serial)(struct platform_intf *intf);
	char *(*system_sku)(struct platform_intf *intf);
	char *(*google_hwqualid)(struct platform_intf *intf);
};

/* system information callbacks */
struct mce;
struct edac_event;
struct gtune_var;
struct sys_cb {
	/* methods useful for probing */
	char *(*vendor)(struct platform_intf *intf);
	char *(*name)(struct platform_intf *intf);
	char *(*version)(struct platform_intf *intf);
	char *(*family)(struct platform_intf *intf);
	char *(*variant)(struct platform_intf *intf);

	/* Obtain the model name of this device. With unified builds a board
	 * can support multiple models. This allows the model to be obtained,
	 * which can be used to adjust how various packages work. */
	char *(*model)(struct platform_intf *intf);

        /* Query an identifier for chassis. */
        char *(*chassis)(struct platform_intf *intf);

	/* firmware info */
	char *(*firmware_vendor)(struct platform_intf *intf);
	char *(*firmware_version)(struct platform_intf *intf);

	/* Boot-time initialization. This may include things the BIOS / kernel
	   did not initialize.  */
	int (*init_platform)(struct platform_intf *intf);

	/* read and write platform-specific settings */
	int (*print_settings)(struct platform_intf *intf);

	/* custom reset function */
	int (*reset)(struct platform_intf *intf);

#if 0
	int (*tune_settings)(struct platform_intf *intf, int argc, char **argv);
	/* Machine Check Exceptions */
	int (*mce_handler)(struct platform_intf *intf, struct mce *mce);
	int (*mce_printall)(struct platform_intf *intf);
	/* Error Detection And Correction */
	int (*edac_setup)(struct platform_intf *intf);
	int (*edac_handler)(struct platform_intf *intf,
	                    struct edac_event *event);
#endif
#if 0
	/* FIXME: these should go into another set of callbacks, or something */
	/* CPU enumeration and information */
	int (*get_cores_per_socket)(struct platform_intf *intf);
	int (*get_logical_cores_enabled)(struct platform_intf *intf);
	int (*get_physical_cores_enabled)(struct platform_intf *intf);
	int (*get_sockets_enabled)(struct platform_intf *intf);
	int (*query_socket)(struct platform_intf *intf, int num);
	uint8_t (*get_microcode)(struct platform_intf *intf, int cpu);
//	enum cpu_types (*get_cpu_type)(struct platform_intf *intf, int node);
#endif
#if 0
	/* Tunable variable list */
	struct gtune_var **gtune_var_list;
#endif
};

/* flash callbacks */
struct flash_cb {
	int (*dump)(struct platform_intf *intf, const char *file);
	int (*verify)(struct platform_intf *intf, const char *file);
	int (*print_map)(struct platform_intf *intf, const char *file);
	int (*print_checksum)(struct platform_intf *intf, const char *file);
	int (*size)(struct platform_intf *intf);
};

/* gpio callbacks */
struct gpio_cb {
	/* read level for a specific GPIO */
	int (*read)(struct platform_intf *intf, struct gpio_map *gpio);
	/* get mapping for a specific GPIO */
	struct gpio_map *(*map)(struct platform_intf *intf, const char *name);
	/* list all GPIOs for the platform */
	int (*list)(struct platform_intf *intf);
	/* set a specific GPIO state to 0 or 1 */
	int (*set)(struct platform_intf *intf, const char *name, int state);
};

/* fru callbacks */
struct fru_cb {
	int (*info)(struct platform_intf *intf, struct eeprom *eeprom);
	int (*verify)(struct platform_intf *intf, struct eeprom *eeprom);
	int (*create)(struct platform_intf *intf, struct eeprom *eeprom,
	              char *mfg, char *prod, char *serial, char *part,
	              char *timestamp);
	int (*read)(struct platform_intf *intf,
	            struct eeprom *eeprom, const char *fname);
	int (*write)(struct platform_intf *intf,
	             struct eeprom *eeprom, const char *fname);
};

/* "legacy" ec callbacks (before support for multiple ECs was introduced) */
struct legacy_ec_cb {
	const char *(*vendor)(struct platform_intf *intf);
	const char *(*name)(struct platform_intf *intf);
	const char *(*fw_version)(struct platform_intf *intf);

	int (*setup)(struct platform_intf *intf);
	int (*destroy)(struct platform_intf *intf);

	void *priv;	/* private data for EC */
};

struct ec_cb {
	const char *(*vendor)(struct platform_intf *intf, struct ec_cb *ec);
	const char *(*name)(struct platform_intf *intf, struct ec_cb *ec);
	const char *(*fw_version)(struct platform_intf *intf, struct ec_cb *ec);

	int (*setup)(struct platform_intf *intf, struct ec_cb *ec);
	int (*destroy)(struct platform_intf *intf, struct ec_cb *ec);

	void *priv;	/* private data for EC */
};

/* hid touchpad callbacks */
struct hid_tp_cb {
	const char *(*vendor)(struct platform_intf *intf);
	const char *(*name)(struct platform_intf *intf);
	const char *(*fw_version)(struct platform_intf *intf);
	const char *(*hw_version)(struct platform_intf *intf);
};

/* hid callbacks */
struct hid_cb {
	struct hid_tp_cb *tp;
};

/* pci callbacks */
struct pci_function_info;
struct pci_cb {
	const struct pci_entity_info * (*lookup)(struct platform_intf * intf,
	                                         int bus, int dev, int func);
};

/* battery related callbacks */
struct battery_cb {
	const char *(*get_fud)(struct platform_intf *intf);
	int (*set_fud)(struct platform_intf *intf,
			int day, int month, int year);
	int (*update)(struct platform_intf *intf);
};

enum psu_types {
	PSU_TYPE_UNKNOWN,
	PSU_TYPE_BATTERY,	/* AC + rechargeable battery */
	PSU_TYPE_AC_ONLY,	/* No battery in system */
	PSU_TYPE_AC_PRIMARY,	/* AC as primary source + backup battery */
};

/* power supply related callbacks */
struct psu_cb {
	enum psu_types (*type)(struct platform_intf *intf);
};

/* storage device callbacks */
struct storage_cb {
	const char *(*get_model_name)(struct platform_intf *intf);
	enum storage_phy_speed (*get_phy_speed)(struct platform_intf *intf);
	int (*set_phy_speed)(struct platform_intf *intf,
			     enum storage_phy_speed phy_speed);
};

/* platform-specific callbacks */
struct platform_cb {
	struct sensor_cb *sensor;	/* sensor callbacks */
	struct memory_cb *memory;	/* memory callbacks */
	struct eventlog_cb *eventlog;	/* eventlog callbacks */
	struct bootnum_cb *bootnum;	/* boot number callbacks */
	struct smbios_cb *smbios;	/* smbios related callbacks */
	struct sys_cb *sys;		/* system callbacks */
	struct flash_cb *flash;		/* flash related callbacks */
	struct nvram_cb *nvram;		/* nvram related callbacks */
	struct gpio_cb *gpio;		/* gpio callbacks */
	struct eeprom_cb *eeprom;	/* eeprom callbacks */
	struct fru_cb *fru;		/* fru callbacks */
	struct pci_cb *pci;		/* pci callbacks */
	struct vpd_cb *vpd;		/* vpd callbacks */
	struct ec_cb *ec;		/* ec callbacks */
	struct ec_cb *pd;		/* pd callbacks */
	struct ec_cb *sh;		/* sh callbacks */
	struct legacy_ec_cb *legacy_ec;	/* legacy ec callbacks */
	struct hid_cb *hid;		/* hid callbacks */
	struct battery_cb *battery;	/* battery callbacks */
	struct storage_cb *storage;	/* storage callbacks */
	struct psu_cb *psu;		/* power supply callbacks */
};

/*
 * Top-level interface handler.
 * One of these should be defined for each supported platform.
 */
struct platform_intf {
	enum platform_type type;	/* numeric platform type */
	const char *name;		/* canonical platform name */
	const char **id_list;		/* list of supported ids */
	struct platform_cmd **sub;	/* list of commands */
	struct platform_op *op;		/* operations */
	struct platform_cb *cb;		/* callbacks */

	/*
	 * returns 1 to indicate platform identified
	 * returns 0 otherwise
	 */
	int (*probe)(struct platform_intf *intf);
	int (*setup)(struct platform_intf *intf);
	int (*setup_post)(struct platform_intf *intf);
	int (*destroy)(struct platform_intf *intf);
	const char *version_id;		/*platform board id */
};

/* The global list of all platforms. */
extern struct platform_intf *platform_intf_list[];

/*
 * mosys_platform_setup  -  determine current platform and return handler
 *
 * returns pointer to pre-defined interface for detected platform
 * returns NULL if platform not identified or other error
 *
 */
extern struct platform_intf *mosys_platform_setup(const char *p_opt);

/*
 * mosys_platform_destroy  -  clean up platform interface
 *
 * @intf:	platform interface
 *
 */
extern void mosys_platform_destroy(struct platform_intf *intf);

/*
 * platform_cmd_usage  -  print usage help text for command
 *
 * @cmd:	command pointer
 *
 */
extern void platform_cmd_usage(struct platform_cmd *cmd);

/*
 * print_tree - print command tree for this platform
 *
 * @intf:	platform interface
 */
extern void print_tree(struct platform_intf *intf);

extern int print_platforms(void);

#endif /* MOSYS_PLATFORM_H__ */
