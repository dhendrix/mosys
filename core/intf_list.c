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
