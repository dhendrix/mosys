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

#ifndef MOSYS_PLATFORM_H__
#define MOSYS_PLATFORM_H__

#include "mosys/mosys.h"

struct kv_pair;

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
	int (*list_thermal)(struct platform_intf *intf);
	int (*list_voltage)(struct platform_intf *intf);
	int (*list_fantach)(struct platform_intf *intf);
	int (*list_current)(struct platform_intf *intf);
	int (*list_power)(struct platform_intf *intf);
	int (*set_fantach)(struct platform_intf *intf,
			   const char *name, const char *spec);
	int (*set_voltage)(struct platform_intf *intf, const char *name, 
			   const char *param, double val);
	int (*set_thermal)(struct platform_intf *intf, const char *name,
			   const char *param, double val);
	struct sensor_array *(*get_sensors)(struct platform_intf *intf);
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
	int (*dimm_spd)(struct platform_intf *intf,
	                int dimm, unsigned char *buf);
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
};

/* eventlog clearing and status */
enum eventlog_clear_type {
	EVENTLOG_CLEAR_STATUS = 0,
	EVENTLOG_CLEAR_25 = 25,
	EVENTLOG_CLEAR_50 = 50,
	EVENTLOG_CLEAR_75 = 75,
	EVENTLOG_CLEAR_100 = 100,
};

/* eventlog related callbacks */
struct smbios_log_entry;
struct eventlog_cb {
	int (*print_multi)(struct platform_intf *intf,
	                   struct smbios_log_entry *entry, int start_id);
	int (*print_type)(struct platform_intf *intf,
	                  struct smbios_log_entry *entry,
	                  struct kv_pair *kv);
	int (*print_data)(struct platform_intf *intf,
	                  struct smbios_log_entry *entry,
	                  struct kv_pair *kv);
	int (*verify)(struct platform_intf *intf,
	              struct smbios_log_entry *entry);
	int (*add)(struct platform_intf *intf, int argc, char **argv);
	int (*clear)(struct platform_intf *intf,
	             enum eventlog_clear_type type);
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

/* smbios callbacks */
struct smbios_cb {
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
struct sysinfo_cb {
	/* methods useful for probing */
	const char *(*vendor)(struct platform_intf *intf);
	const char *(*name)(struct platform_intf *intf);
	const char *(*version)(struct platform_intf *intf);
	const char *(*family)(struct platform_intf *intf);
	const char *(*variant)(struct platform_intf *intf);

	/* Boot-time initialization. This may include things the BIOS / kernel
	   did not initialize.  */
	int (*init_platform)(struct platform_intf *intf);

	/* read and write platform-specific settings */
	int (*print_settings)(struct platform_intf *intf);
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

/* ec callbacks */
struct ec_cb {
	const char *(*vendor)(struct platform_intf *intf);
	const char *(*name)(struct platform_intf *intf);
	const char *(*fw_version)(struct platform_intf *intf);
};

/* pci callbacks */
struct pci_function_info;
struct pci_cb {
	const struct pci_entity_info * (*lookup)(struct platform_intf * intf,
	                                         int bus, int dev, int func);
};

/* platform-specific callbacks */
struct platform_cb {
	struct sensor_cb *sensor; 	/* sensor callbacks */
	struct memory_cb *memory; 	/* memory callbacks */
	struct eventlog_cb *eventlog; 	/* eventlog callbacks */
	struct bootnum_cb *bootnum; 	/* boot number callbacks */
	struct smbios_cb *smbios;	/* smbios related callbacks */
	struct sysinfo_cb *sysinfo; 	/* system info callbacks */
	struct flash_cb *flash; 	/* flash related callbacks */
	struct nvram_cb *nvram; 	/* nvram related callbacks */
	struct gpio_cb *gpio;		/* gpio callbacks */
	struct eeprom_cb *eeprom;	/* eeprom callbacks */
	struct fru_cb *fru;		/* fru callbacks */
	struct pci_cb *pci;		/* pci callbacks */
	struct vpd_cb *vpd;		/* vpd callbacks */
	struct ec_cb *ec;		/* ec callbacks */
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
