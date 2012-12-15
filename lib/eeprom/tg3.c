/* Copyright 2012, Google Inc.
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
 *
 * FIXME: much of this was scrubbed... might want to eventually figure out
 * what is public so we can make this more useful
 */

#if defined (__linux__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mosys/platform.h"
#include "mosys/globals.h"
#include "mosys/alloc.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"

#include "lib/string.h"

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
	int i = 0, rc = 0;

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
			rc = -1;
			break;
		}

		/* Always print raw data last */
		str = buf2str(ee->data + f->offset, f->len);
		kv_pair_fmt(kv, "raw", "0x%s", str);

		rc = kv_pair_print(kv);
		kv_pair_free(kv);

		free(val);
		free(str);
		if (rc)
			break;
	}

	return rc;
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

#endif	/* __linux__ */
