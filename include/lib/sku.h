/**
 * Copyright 2017 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MOSYS_LIB_SKU_H__
#define MOSYS_LIB_SKU_H__

struct platform_intf;

enum {
	SKU_NUMBER_ANY = -1,  /* Matches any SKU number. */
};

/**
 * sku_info - a structure to hold per-SKU information.
 *
 * `brand` is the 4 letter RLZ brand code (go/crosrlz), different for each LOEM
 *         in White Label devices, and also used for CR50 BoardID.
 * `model` is the model name (the public code name on Golden Eye Device list).
 * `chassis` is the chassis ID (usually same as model in lower case).
 * `customization` is the legacy support for customization_id.
 * `data` is a general pointer for platform implementations to use, for example
 *         adding peripheral status like number of cameras or form factor.
 */
struct sku_info {
	const char *brand;
	const char *model;
	const char *chassis;
	const char *customization;
	void *data;
};

/**
 * sku_mapping - a mapping from SKU number to sku_info.
 *
 * `number` is an integer. Use SKU_NUMBER_ANY to match all devices.
 * `info` points to a sku_info entry. NULL to indicate end of entries.
 */
struct sku_mapping {
	int number;
	const struct sku_info *info;
};

/**
 * sku_get_number - get SKU number
 *
 * returns integer representing SKU number
 * returns -1 to indicate value not found or error
 */
extern int sku_get_number(struct platform_intf *intf);

/**
 * sku_find_info - find matched SKU info from a list of mappings.
 *
 * returns a pointer to matched sku_info record
 * returns NULL to indicate SKU not found or error
 */
extern const struct sku_info *sku_find_info(struct platform_intf *intf,
					    const struct sku_mapping *mappings);


/**
 * sku_get_brand - get (RLZ) brand code
 *
 * returns allocated string containing value if found
 * returns NULL to indicate value not found or error
 */
extern char *sku_get_brand(struct platform_intf *intf);

/**
 * sku_get_chassis - get chassis id
 *
 * returns allocated string containing value if found
 * returns NULL to indicate value not found or error
 */
extern char *sku_get_chassis(struct platform_intf *intf);

/**
 * sku_get_customization - get customization ID
 *
 * returns allocated string containing value if found
 * returns NULL to indicate value not found or error
 */
extern char *sku_get_customization(struct platform_intf *intf);

/**
 * sku_get_model - get model name
 *
 * returns allocated string containing value if found
 * returns NULL to indicate value not found or error
 */
extern char *sku_get_model(struct platform_intf *intf);

/**
 * sku_get_data - get SKU specific data
 *
 * returns a pointer to the `data` member in `sku_info`.
 * returns NULL to indicate value not found or error
 */
extern void *sku_get_data(struct platform_intf *intf);


#endif /* MOSYS_LIB_SKU_H__ */
