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

#ifndef MOSYS_LIB_PROBE_H__
#define MOSYS_LIB_PROBE_H__

struct platform_intf;

/*
 * probe_hwid - attempt to match chromeos hardware id
 *
 * @hwids:	null-terminated list of hardware IDs
 *
 * returns 1 to indicate match
 * returns 0 to indicate no match
 * returns <0 to indicate error
 */
extern int probe_hwid(const char *hwids[]);

/*
 * probe_frid - attempt to match platform to chromeos firmware revision id
 *
 * @hwids:	null-terminated list of hardware IDs
 *
 * returns 1 to indicate match
 * returns 0 to indicate no match
 * returns <0 to indicate error
 */
extern int probe_frid(const char *hwids[]);

/*
 * probe_smbios - probe smbios for system info
 *
 * @ids:	null-terminated list of ids
 *
 * returns 1 to indicate match
 * returns 0 to indicate no match
 * returns <0 to indicate error
 */
extern int probe_smbios(struct platform_intf *intf, const char *ids[]);

/*
 * probe_cpuinfo - probe /proc/cpuinfo for system info
 *
 * @key:	key to search for
 * @value:	value to search for
 *
 * This function assumes the format is colon-delimited with unknown number of
 * spaces between the key, colon, and value. For example
 * key     : value
 *
 * returns 1 to indicate matching key:value pair found
 * returns 0 to indicate no matching key:value pair found (clean exit)
 * returns <0 to indicate error
 */
extern int probe_cpuinfo(struct platform_intf *intf,
                         const char *key, const char *value);

/*
 * extract_cpuinfo - extract value from /proc/cpuinfo
 *
 * @key:	key to search for
 *
 * This function assumes the format is colon-delimited with unknown number of
 * spaces between the key, colon, and value. For example
 * key     : value
 *
 * returns allocated string containing value if found
 * returns NULL to indicate value not found or error
 */
extern const char *extract_cpuinfo(const char *key);

/*
 * extract_block_device_model_name - extract block device name from sysfs
 *
 * @device:	device name to extract (ex. "sda")
 *
 * returns allocated string containing value if found
 * returns NULL to indicate value not found or error
 */
extern const char *extract_block_device_model_name(const char *device);

/*
 * probe_cmdline - probe /proc/cmdline for key
 *
 * @key:	key to search for
 * @cs:		case-sensitivity
 *
 * returns 1 to indicate matching key found
 * returns 0 to indicate no matching key pair found (clean exit)
 * returns <0 to indicate error
 */
extern int probe_cmdline(const char *key, int cs);

#endif /* MOSYS_LIB_PROBE_H__ */
