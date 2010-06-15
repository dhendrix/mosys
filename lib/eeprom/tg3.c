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

/* FIXME: much of this was scrubbed... might want to eventually figure out
 * what is public so we can make this more useful */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mosys/platform.h"
#include "mosys/globals.h"
#include "mosys/alloc.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/string.h"

#include "ethtool-util.h"

struct enet_nvram_field {
	const char *name;
	int verbose;
	unsigned int offset;
	unsigned int len;
	/* For fields that have useful information contained in sub-fields */
	struct kv_pair *(*decode)(struct platform_intf *intf,
				  struct enet_nvram_field *field,
				  uint8_t *val, struct kv_pair *kv);
};

static struct kv_pair *print_ascii(struct platform_intf *intf,
				   struct enet_nvram_field *f,
				   uint8_t *val, struct kv_pair *kv) {
	kv_pair_add(kv, "ascii", (char *)val);
	return kv;
}

/* Manufacturing Information */
static struct enet_nvram_field mfginfo_map[] = {
	{ "MAC address", 0, 0x7C, 8, NULL },
	{ "Part number", 0, 0x84, 16, print_ascii },
	{ "Part revision", 0, 0x94, 2, NULL },
	{ "Device ID", 2, 0xA0, 2, NULL },
	{ "Vendor ID", 2, 0xA2, 2, NULL },
	{ "PCI Subsystem Device ID", 2, 0xA4, 2, NULL },                  
	{ "PCI Subsystem Vendor ID", 2, 0xA6, 2, NULL },                  
	{ NULL },
};

/*
 * print_enet_nvram: Iteratively print a table of ethernet nvram registers.
 * The default mode for doing so is to translate raw data into a string and
 * place it in a key-value pair. If a special decoding function is supplied,
 * it will be used instead to generate key-value pairs.
 *
 * @intf:	the platform interface
 * @ifr:	interface information
 * @ee:		the eeprom data
 * @map:	the map to use to translate eeprom data
 *
 * returns 0 if successful
 * returns <0 to indicate failure
 */
static int print_enet_nvram(struct platform_intf *intf,
			    struct ifreq *ifr,
			    struct ethtool_eeprom *ee,
			    struct enet_nvram_field *map) {
	int i = 0;

	while (map[i].name != NULL) {
		uint8_t *val;
		char *str;
		struct kv_pair *kv;
		struct enet_nvram_field *f = &map[i];
	
		/* Do nothing and skip to the next step if this field is
		 * superfluous */
		i++;
		if (mosys_get_verbosity() < f->verbose) {
			continue;
		}
		
		val = mosys_malloc(f->len);
		memcpy(val, ee->data + f->offset, f->len);

		kv = kv_pair_new();
		kv_pair_add(kv, "iface", ifr->ifr_name);
		kv_pair_add(kv, "name", f->name);
		if (f->decode && f->decode(intf, f, val, kv) == NULL) {
			lprintf(LOG_ERR, "%s: error decoding %s",
						__func__, f->name);
			return -1;
		}

		/* Always print raw data last */
		str = buf2str(ee->data + f->offset, f->len);
		kv_pair_fmt(kv, "raw", "0x%s", str);

		kv_pair_print(kv);
		kv_pair_free(kv);

		free(val);
		free(str);
	}

	return 0;
}

/*
 * tg3_dump_eeprom: Entry point for tg3-based register read/decode functions.
 *
 * @intf:	the platform interface
 * @info:	driver info
 * @ifr:	interface information
 * @ee:		eeprom data
 *
 * returns 0 if successful
 * returns <0 to indicate failure
 */
int tg3_dump_eeprom(struct platform_intf *intf,
		    struct ethtool_drvinfo *info,
		    struct ifreq *ifr,
		    struct ethtool_eeprom *ee) {
	int ret = 0;
	
	ret |= print_enet_nvram(intf, ifr, ee, mfginfo_map);

	return ret;
}
