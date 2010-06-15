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
 *
 * pci.c: Quick PCI access, requires linux /proc/bus/pci or
 * directory that has same structure
 *
 * FIXME: does not handle extended configuration space (>256B)
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <dirent.h>

#include "mosys/platform.h"
#include "mosys/globals.h"
#include "mosys/string.h"
#include "mosys/log.h"

#include "intf/pci.h"

/* default PCI root directory */
#define PCI_PROC_DIR	"/proc/bus/pci"

/* used throughout this file */
static char *pci_proc_dir;

static int pci_setup(struct platform_intf *intf)
{
	pci_proc_dir = format_string("%s/%s", mosys_get_root_prefix(),
	                             PCI_PROC_DIR);
	return 0;
}

static void pci_destroy(struct platform_intf *intf)
{
	free(pci_proc_dir);
}

/*
 * pci_open_file  -  Open PCI dev file based on bus/dev/func under given root
 *
 * @bus:	pci bus number
 * @dev:	pci device id
 * @func:	pci function id
 * @rw:         read=0, write=1
 *
 * returns valid file descriptor on success
 * returns <0 on failure
 */
static int pci_open_file(int bus, int dev, int func, int rw)
{
	char pcif[1024];

	snprintf(pcif, sizeof(pcif), "%s/%02x/%02x.%x",
	         pci_proc_dir, bus, dev, func);

	return open(pcif, rw);
}

/*
 * pci_read_file  -  Read from pci configuration space
 *
 * @intf:       platform interface
 * @bus:        pci bus
 * @dev:        pci device
 * @func:       pci function
 * @reg:        configuration register
 * @length:     number of bytes to read
 * @data:       buffer to read data into
 *
 * returns number of bytes read
 * returns <0 to indicate failure
 */
static int pci_read_file(struct platform_intf *intf, int bus,
                         int dev, int func, int reg, int length, void *data)
{
	int fd, rlen;

	lprintf(LOG_DEBUG,
	        "pci_read_file: reading %02x:%02x.%x at %02x (%d bytes)\n", bus,
	        dev, func, reg, length);

	if (length < 1 || length > 255)
		return -1;

	fd = pci_open_file(bus, dev, func, O_RDONLY);
	if (fd < 0) {
		lprintf(LOG_DEBUG,
		        "Unable to open PCI %02x:%02x.%1x for reading\n", bus,
		        dev, func);
		return -1;
	}

	/* seek to starting register */
	if (lseek(fd, reg, SEEK_SET) == (off_t) - 1) {
		lperror(LOG_DEBUG,
		        "Unable to read from PCI %02x:%02x.%1x at 0x%x", bus,
		        dev, func, reg);
		close(fd);
		return -1;
	}

	/* read the data */
	rlen = read(fd, data, length);

	if (rlen != length)
		lperror(LOG_DEBUG, "Unable to read %d bytes from PCI"
		        " %02x:%02x.%1x at 0x%x", length, bus, dev, func, reg);

	close(fd);

	return rlen;
}

/*
 * pci_write_file  -  Write to pci configuration space
 *
 * @intf:	platform interface
 * @bus:	pci bus
 * @dev:	pci device
 * @func:	pci function
 * @reg:	configuration register
 * @size:	number of bytes to read
 * @data:       buffer to write from
 *
 * returns number of bytes written
 * returns <0 to indicate failure
 */
static int pci_write_file(struct platform_intf *intf, int bus, int dev,
                          int func, int reg, int length, const void *data)
{
	int fd, wlen;

	lprintf(LOG_DEBUG,
	        "pci_write_file: writing %02x:%02x.%x at %02x (%d bytes)\n",
	        bus, dev, func, reg, length);

	if (length < 1 || length > 256)
		return -1;

	fd = pci_open_file(bus, dev, func, O_WRONLY);
	if (fd < 0) {
		lprintf(LOG_DEBUG,
		        "Unable to open PCI %02x:%02x.%1x for writing\n", bus,
		        dev, func);
		return -1;
	}

	/* seek to starting register */
	if (lseek(fd, reg, SEEK_SET) == (off_t) - 1) {
		lperror(LOG_DEBUG, "Unable to write to PCI %02x:%02x.%1x at 0x%x",
		        bus, dev, func, reg);
		close(fd);
		return -1;
	}

	/* write the data */
	wlen = write(fd, data, length);

	if (wlen != length)
		lperror(LOG_DEBUG, "Unable to write %d bytes to PCI"
		        " %02x:%02x.%1x at 0x%x", length, bus, dev, func, reg);

	close(fd);

	return wlen;
}

/*
 * pci_do_foreach_in_bus  -  Execute function on all PCI devices on a given bus
 *
 * @intf:	platform interface
 * @bus:	target pci bus
 * @cb:		pci callback
 * @data:	data supplied to callback
 *
 * Supplied PCI callback will cause bus enumeration to stop if
 * it returns non-zero value.
 *
 * returns 0 to indicate success, failure otherwise.
 */
static int pci_do_foreach_in_bus(struct platform_intf *intf, int bus,
                                 pci_callback_t cb, void *data)
{
	struct dirent *d;
	char path[1024];
	DIR *dp;
	int dev = 0, func = 0;
	int ret = 0;

	if (cb == NULL)
		return -1;

	/* directory is a bus number */
	snprintf(path, sizeof(path), "%s/%02x/",
		 pci_proc_dir, bus);

	if (!(dp = opendir(path))) {
		lprintf(LOG_DEBUG, "Failed to open %s\n", path);
		return -1;
	}

	while ((d = readdir(dp))) {
		if (d->d_type != DT_REG)
			continue;

		/* file is a pci device.function */
		if (sscanf(d->d_name, "%02x.%01x", &dev, &func) != 2) {
			lprintf(LOG_DEBUG, "Invalid PCI file name: %s/%s\n",
			        path, d->d_name);
			continue;
		}

		/* now exec callback, non-zero return means stop */
		lprintf(LOG_DEBUG,
		        "Executing callback for device %02x:%02x.%01x\n",
		        bus, dev, func);

		ret = cb(intf, bus, dev, func, data);
		if (ret != 0) {
			break;
		}
	}
	closedir(dp);
	return ret;
}

/*
 * pci_do_foreach  -  Execute function on all PCI devices
 *
 * @intf:	platform interface
 * @cb:		pci callback
 * @data:	data supplied to callback
 *
 * Supplied PCI callback will cause bus enumeration to stop if
 * it returns non-zero value.
 *
 * returns 0 to indicate success, failure otherwise.
 */
static int pci_do_foreach(struct platform_intf *intf,
                          pci_callback_t cb, void *data)
{
	struct dirent *topd;
	DIR *topdp;
	int ret = 0;

	if (!(topdp = opendir(pci_proc_dir))) {
		lprintf(LOG_DEBUG, "Failed to open %s\n", pci_proc_dir);
		return -1;
	}

	while ((topd = readdir(topdp))) {
		if (topd->d_type != DT_DIR)
			continue;

		/* Ignore driectories with '.' in the first part of the name. */
		if (topd->d_name[0] == '.')
			continue;

		ret = pci_foreach_in_bus(intf,
		                         strtol(topd->d_name, NULL, 16), cb, data);

		if (ret != 0) {
			closedir(topdp);
			return ret;
		}
	}

	if (topdp)
		closedir(topdp);

	return ret;
}

/* PCI operations based on /proc directory */
struct pci_intf pci_file_intf = {
	.setup		= pci_setup,
	.destroy	= pci_destroy,
	.read		= pci_read_file,
	.write		= pci_write_file,
	.foreach	= pci_do_foreach,
	.foreach_in_bus	= pci_do_foreach_in_bus,
};
