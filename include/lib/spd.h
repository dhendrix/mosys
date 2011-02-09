/*
 * Copyright (C) 2011 Google Inc.
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
 *
 * Serial Presence Detect (SPD) code for access of SPDs on DIMMs.
 */

#ifndef LIB_SPD_H__
#define LIB_SPD_H__

#include <inttypes.h>

#include "intf/i2c.h"

#define SPD_READ          0
#define SPD_WRITE         1
#define SPD_MAX_LENGTH    256

/* forward declarations */
struct kv_pair;
struct platform_intf;
struct spd_reg;

/* different types for SPD */
enum spd_type {
	SPD_TYPE_DDR = 0x07,
	SPD_TYPE_DDR2 = 0x08,
	SPD_TYPE_FBDIMM = 0x09,
	SPD_TYPE_DDR3 = 0x0b,
};

/* spd register handlers */
struct spd_reg {
	const char *name;
	const char *units;
	const char *(*func)(struct spd_reg *reg,
			    const uint8_t *eeprom,
			    uint8_t byte);
	const char *table[256];
};

struct spd_callbacks {
	enum spd_type type;
	struct spd_reg *regs;
	int num_regs;
};

//extern struct spd_callbacks ddr1_callbacks;
extern struct spd_callbacks ddr2_callbacks;
//extern struct spd_callbacks fbdimm_callbacks;

struct spd_eeprom {
	int length;
	uint8_t data[SPD_MAX_LENGTH];
};

struct spd_device {
	int dimm_num; /* DIMM number in system. */
	enum spd_type type; /* SPD type. */
	struct i2c_addr smbus; /* Address of DIMM in system. */
	struct spd_eeprom eeprom;
};

/*
 * various SPD fields that can be retrieved
 * these are found in different locations on DDR/DDR2 vs. FBDIMM
 */
enum spd_field_type {
	SPD_GET_MFG_ID,		/* Module Manufacturer ID */
	SPD_GET_MFG_ID_DRAM,	/* DRAM Manufacturer ID */
	SPD_GET_MFG_LOC,	/* Module Manufacturing Location */
	SPD_GET_MFG_DATE,	/* Module Manufacturing Date */
	SPD_GET_SERIAL_NUMBER,	/* Module Serial Number */
	SPD_GET_PART_NUMBER,	/* Module Part Number */
	SPD_GET_REVISION_CODE,	/* Module Revision Code */
	SPD_GET_SIZE,		/* Module Size (in MB) */
	SPD_GET_ECC,		/* ECC capable: boolean */
	SPD_GET_RANKS,		/* Number of ranks */
	SPD_GET_WIDTH,		/* SDRAM device width */
	SPD_GET_CHECKSUM,	/* SPD checksum */
	SPD_GET_SPEEDS,		/* module frequency capabilities */
};

/*
 * new_spd_device() - create a new instance of spd_device
 *
 * @intf:  platform_intf for access
 * @dimm:  Google logical dimm number to represent
 *
 * returns allocated and filled in spd_devices on success, NULL if error
 */
extern struct spd_device *new_spd_device(struct platform_intf *intf, int dimm);

/* add register to key=value pair */
extern int spd_print_reg(struct platform_intf *intf,
			 struct kv_pair *kv, const void *data, uint8_t reg);

/* add field to key=value pair */
extern int spd_print_field(struct platform_intf *intf,
			   struct kv_pair *kv,
			   const void *data, enum spd_field_type type);

/* print raw spd */
extern int spd_print_raw(struct kv_pair *kv, int len, uint8_t *data);

/*
 * spd_read_i2c  -  Read from SPD configuration space
 *
 * @intf:	platform interface
 * @bus:	i2c bus
 * @address:	i2c address
 * @reg:	register offset
 * @length:	number of bytes to read
 * @data:       data buffer
 *
 * returns number of bytes read
 * returns <0 to indicate error
 */
extern int spd_read_i2c(struct platform_intf *intf, int bus,
                        int addr, int reg, int length, void *data);

#if 0
/*
 * spd_write_i2c  -  Write to SPD configuration space
 *
 * @intf:	platform interface
 * @bus:	i2c bus
 * @address:	i2c address
 * @reg:	register offset
 * @length:	number of bytes to write
 * @data:       data buffer
 *
 * returns number of bytes written
 * returns <0 to indicate error
 */
extern int spd_write_i2c(struct platform_intf *intf, int bus,
                         int addr, int reg, int length, const void *data);
#endif

/* spd_raw_access - read/write access method to SPDs
 *
 * @intf:  platform interface
 * @bus:  SMBus number of requested SPD
 * @address: SMBus address of requested SPD
 * @reg:  register in SPD to perform access on
 * @length:  length of access to perform
 * @data: pointer to buffer that is either filled or read from to do the access
 * @rw:  specify SPD operation (SPD_READ or SPD_WRITE)
 *
 * returns 0 on success, < 0 on error.
 */
extern int spd_raw_access(struct platform_intf *intf, int bus, int address,
                          int reg, int length, void *data, int rw);

/* Introduce ability to override SPD raw access operation. Provide a type for
 * smaller function signatures. */
typedef int (*spd_raw_override)(struct platform_intf *intf, int bus,
                                int address, int reg, int length, void *data,
                                int rw);

/* override_spd_raw_access - override the spd_raw() function.
 *
 * @override:  function to invoke instead of the normal spd_raw() path.
 *
 * return 0 on success, < 0 otherwise.
 */
extern int override_spd_raw_access(spd_raw_override override);

/*
 * spd_total_size  -  determine total bytes in spd from first few bytes
 *
 * @data:	spd data
 *
 * returns total size of SPD, may be less than max depending on type of module
 * returns <0 to indicate failure
 *
 */
extern int spd_total_size(uint8_t *data);

/*
 * SPD register and field callbacks.
 */

/* Common register printing callbacks. */
extern const char *spd_revision_code(struct spd_reg *reg,
                                     const uint8_t * eeprom, uint8_t byte);
extern const char *spd_shift_by(struct spd_reg *reg,
                                const uint8_t * eeprom, uint8_t byte);
extern const char *spd_module_bank_density(struct spd_reg *reg,
                                           const uint8_t * eeprom,
                                           uint8_t byte);
extern const char *spd_shift_access_time(struct spd_reg *reg,
                                         const uint8_t * eeprom, uint8_t byte);
extern const char *spd_decimal_access_time(struct spd_reg *reg,
                                           const uint8_t * eeprom,
                                           uint8_t byte);
extern const char *spd_burst(struct spd_reg *reg, const uint8_t * eeprom,
                             uint8_t byte);
extern const char *spd_readhex(struct spd_reg *reg,
                               const uint8_t * eeprom, uint8_t byte);
extern const char *spd_readbyte(struct spd_reg *reg,
                                const uint8_t * eeprom, uint8_t byte);
extern const char *spd_table_lookup(struct spd_reg *reg,
                                    const uint8_t * eeprom, uint8_t byte);

#if 0
/*
 * spd_print_field_ddr1  -  add common DDR SPD fields into key=value pair
 *
 * @intf:       platform interface
 * @kv:         key=value pair
 * @data:       raw spd data
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
extern int spd_print_field_ddr1(struct platform_intf *intf, struct kv_pair *kv,
                                const void *data, enum spd_field_type type);
#endif

/*
 * spd_print_field_ddr2  -  add common DDR SPD fields into key=value pair
 *
 * @intf:       platform interface
 * @kv:         key=value pair
 * @data:       raw spd data
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
extern int spd_print_field_ddr2(struct platform_intf *intf, struct kv_pair *kv,
                                const void *data, enum spd_field_type type);

#if 0
/*
 * spd_print_field_fbdimm  -  add common FDDIMM SPD fields into key=value pair
 *
 * @intf:       platform interface
 * @kv:         key=value pair
 * @data:       raw spd data
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
extern int spd_print_field_fbdimm(struct platform_intf *intf,
                                  struct kv_pair *kv, const void *data,
                                  enum spd_field_type type);
#endif

/*
 * spd_print_field_ddr3  -  add common DDR SPD fields into key=value pair
 *
 * @intf:       platform interface
 * @kv:         key=value pair
 * @data:       raw spd data
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
extern int spd_print_field_ddr3(struct platform_intf *intf, struct kv_pair *kv,
                                const void *data, enum spd_field_type type);

#endif /* LIB_SPD_H__ */
