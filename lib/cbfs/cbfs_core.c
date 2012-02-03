/*
 * This file was originally part of the libpayload project, which is part
 * of coreboot.
 *
 * Portions Copyright (C) 2012 Google Inc.
 * Copyright (C) 2011 secunet Security Networks AG
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Note: This is a modified version of the original cbfs_core.c file from
 * Coreboot. The major changes are intended to remove certain Coreboot-specific
 * dependencies and assumptions, such as callbacks used by Coreboot at runtime,
 * using a supplied memory buffer rather than looking at physical memory,
 * virtual/physical address handling, error/logging macros, and certain constants.
 */

#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#include "mosys/log.h"

#include "lib/cbfs_core.h"
#include "lib/math.h"

struct cbfs_header *get_cbfs_header(const uint8_t *buf, size_t size)
{
	struct cbfs_header *header = NULL;
	uint32_t *header_ptr = NULL;

	/*
	 * Header pointer would usually be at offset -4 from end of ROM
	 * (0xfffffffc on normal x86 systems) and point to an absolute address
	 * somewhere in the Coreboot bootblock section between 0xFFFFB000
	 * and 0xFFFFFFF0.
	 */
	header_ptr = (uint32_t *)((buf + size) - 4);
	header = (struct cbfs_header *)((*header_ptr - (0xffffffff - size + 1)) + buf);

	/* find header */
	if (CBFS_HEADER_MAGIC != ntohl(header->magic)) {
		lprintf(LOG_DEBUG, "Could not find valid CBFS master header at "
		                   "%p: %x vs %x.\n", header, CBFS_HEADER_MAGIC,
				   ntohl(header->magic));
		if (header->magic == 0xffffffff) {
			lprintf(LOG_DEBUG, "Maybe ROM is not mapped properly?\n");
		}
		return (void*)0xffffffff;
	}
	return header;
}

// by must be power-of-two
#define CBFS_ALIGN(val, by) (typeof(val))((unsigned long long)(val + by - 1) & \
		                          (unsigned long long)~(by - 1))
#define CBFS_ALIGN_UP(val, by) CBFS_ALIGN(val + 1, by)

/* public API starts here*/
struct cbfs_file *cbfs_find(const char *name, const uint8_t *buf, size_t size)
{
	struct cbfs_header *header = get_cbfs_header(buf, size);
	void *data, *dataend;
	int align;
	off_t offset = 0;

	if (header == (void*)0xffffffff) return NULL;

	lprintf(LOG_DEBUG, "Searching for %s\n", name);

	/*
	 * Find first entry.
	 * Note: romsize and offset refer to the coreboot image which may
	 * occupy only a small portion of the physical ROM.
	 */
	data = (void *)buf + size - ntohl(header->romsize) + ntohl(header->offset);
	dataend = (void *)buf + size - ntohl(header->bootblocksize);

	align = ntohl(header->align);

	/*
	 * Physical ROM addresses and virtual addresses have different
	 * alignment. Thus, we'll increment by the aligned physical ROM offset
	 * on each iteration rather than working with addresses directly.
	 */
	while ((data < dataend - 1)) {
		struct cbfs_file *file = data + offset;

		if (memcmp(CBFS_FILE_MAGIC, file->magic, strlen(CBFS_FILE_MAGIC)) != 0) {
			offset = CBFS_ALIGN_UP(offset, align);
			continue;
		}
		lprintf(LOG_DEBUG, "%s: Found entry \"%s\" at offset 0x%06x\n",
		                   __func__, CBFS_NAME(file), offset);
		if (strcmp(CBFS_NAME(file), name) == 0) {
			return file;
		}
		offset = CBFS_ALIGN(offset +
		                    ntohl(file->len) + ntohl(file->offset),
		                    align);
	}

	return NULL;
}

void *cbfs_get_file(const char *name, const uint8_t *buf, size_t size)
{
	struct cbfs_file *file = cbfs_find(name, buf, size);

	if (file == NULL) {
		lprintf(LOG_DEBUG, "Could not find file '%s'.\n", name);
		return NULL;
	}

	return (void*)CBFS_SUBHEADER(file);
}

void *cbfs_find_file(const char *name, int type, const uint8_t *buf, size_t size)
{
	struct cbfs_file *file = cbfs_find(name, buf, size);

	if (file == NULL) {
		lprintf(LOG_DEBUG, "Could not find file '%s'.\n", name);
		return NULL;
	}

	if (ntohl(file->type) != type) {
		lprintf(LOG_DEBUG, "File '%s' is of type %x, but we requested %x.\n", name, ntohl(file->type), type);
		return NULL;
	}

	return (void*)CBFS_SUBHEADER(file);
}

int cbfs_decompress(int algo, void *src, void *dst, int len)
{
	switch (algo) {
		case CBFS_COMPRESS_NONE:
			memcpy(dst, src, len);
			return 0;
#ifdef CBFS_CORE_WITH_LZMA
		case CBFS_COMPRESS_LZMA:
			if (ulzma(src, dst) != 0) {
				return 0;
			}
			return -1;
#endif
		default:
			lprintf(LOG_DEBUG, "tried to decompress %d bytes with algorithm #%x, but that algorithm id is unsupported.\n", len, algo);
			return -1;
	}
}

