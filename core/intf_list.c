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
 * intf_list.c: hardware component interfaces
 */

#include "mosys/intf_list.h"
#include "mosys/platform.h"

#include "intf/i2c.h"
#include "intf/io.h"
#include "intf/pci.h"
#include "intf/mmio.h"

#if 0
#include "intf/amb.h"
#include "intf/sp.h"
#include "intf/msr.h"
#include "intf/smi.h"
#include "intf/mce.h"
#include "intf/edac.h"
#include "intf/cpuid.h"
#endif

/* common operations */
struct platform_op platform_common_op = {
	.pci		= &pci_file_intf,
	.i2c		= &i2c_dev_intf,
	.io		= &io_intf,
	.mmio		= &mmio_mmap_intf,
#if 0
	.amb		= &amb_mmio_intf,
	.sp		= &sp_io_intf,
	.msr		= &msr_file_intf,
	.smi		= &smi_gsmi_intf,
	.mce		= &mce_dev_intf,
	.edac		= &edac_sys_intf,
	.cpuid		= &cpuid_sys_intf,
#endif
};

/*
 * intf_op_setup  -  prepare interface operations
 *
 * @intf:       platform interface
 *
 * returns aggregate return code from setup functions
 */
int intf_op_setup(struct platform_intf *intf)
{
	int rc = 0;

	if (intf->op->pci && intf->op->pci->setup)
		rc |= intf->op->pci->setup(intf);
	if (intf->op->i2c && intf->op->i2c->setup)
		rc |= intf->op->i2c->setup(intf);
	if (intf->op->io && intf->op->io->setup)
		rc |= intf->op->io->setup(intf);
	if (intf->op->mmio && intf->op->mmio->setup)
		rc |= intf->op->mmio->setup(intf);
#if 0
	if (intf->op->amb && intf->op->amb->setup)
		rc |= intf->op->amb->setup(intf);
	if (intf->op->sp && intf->op->sp->setup)
		rc |= intf->op->sp->setup(intf);
	if (intf->op->msr && intf->op->msr->setup)
		rc |= intf->op->msr->setup(intf);
	if (intf->op->smi && intf->op->smi->setup)
		rc |= intf->op->smi->setup(intf);
	if (intf->op->cpuid && intf->op->cpuid->setup)
		rc |= intf->op->cpuid->setup(intf);
	/* interfaces to skip when mosys is being used as a library */
	if (!MOSYS_AS_LIBRARY) {
		if (intf->op->mce && intf->op->mce->setup)
			rc |= intf->op->mce->setup(intf);
		if (intf->op->edac && intf->op->edac->setup)
			rc |= intf->op->edac->setup(intf);
	}
#endif

	return rc;
}

/*
 * intf_op_destroy  -  cleanup interface operations
 *
 * @intf:       platform interface
 */
void intf_op_destroy(struct platform_intf *intf)
{
	if (intf->op->pci && intf->op->pci->destroy)
		intf->op->pci->destroy(intf);
	if (intf->op->i2c && intf->op->i2c->destroy)
		intf->op->i2c->destroy(intf);
	if (intf->op->io && intf->op->io->destroy)
		intf->op->io->destroy(intf);
	if (intf->op->mmio && intf->op->mmio->destroy)
		intf->op->mmio->destroy(intf);
#if 0
	if (intf->op->amb && intf->op->amb->destroy)
		intf->op->amb->destroy(intf);
	if (intf->op->sp && intf->op->sp->destroy)
		intf->op->sp->destroy(intf);
	if (intf->op->msr && intf->op->msr->destroy)
		intf->op->msr->destroy(intf);
	if (intf->op->smi && intf->op->smi->destroy)
		intf->op->smi->destroy(intf);
	if (intf->op->cpuid && intf->op->cpuid->destroy)
		intf->op->cpuid->destroy(intf);
	/* interfaces to skip when mosys is being used as a library */
	if (!MOSYS_AS_LIBRARY) {
		if (intf->op->mce && intf->op->mce->destroy)
			intf->op->mce->destroy(intf);
		if (intf->op->edac && intf->op->edac->destroy)
			intf->op->edac->destroy(intf);
	}
#endif
}
