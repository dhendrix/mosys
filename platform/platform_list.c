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
 * All you should need to do to link in a new platform is to hook it in
 * here.
 */
#include <stdlib.h>

#include "mosys/platform.h"

/* default */
extern struct platform_intf platform_default_x86;

/* experimental */
extern struct platform_intf platform_aebl_tegra2;
extern struct platform_intf platform_kaen_tegra2;
extern struct platform_intf platform_seaboard_tegra2;

/* production platforms */
extern struct platform_intf platform_acer_chromia700;
extern struct platform_intf platform_google_cr48;
extern struct platform_intf platform_hp_z600;
extern struct platform_intf platform_samsung_series5;

struct platform_intf *platform_intf_list[] = {
#ifdef CONFIG_ACER_CHROMIA700
	&platform_acer_chromia700,
#endif
#ifdef CONFIG_GOOGLE_CR48
	&platform_google_cr48,
#endif
#ifdef CONFIG_HP_Z600
	&platform_hp_z600,
#endif
#ifdef CONFIG_SAMSUNG_SERIES5
	&platform_samsung_series5,
#endif

/* experimental platforms */
#ifdef CONFIG_EXPERIMENTAL_AEBL
	&platform_aebl_tegra2,
#endif
#ifdef CONFIG_EXPERIMENTAL_KAEN
	&platform_kaen_tegra2,
#endif
#ifdef CONFIG_EXPERIMENTAL_SEABOARD
	&platform_seaboard_tegra2,
#endif

/* place default platform last */
#ifdef CONFIG_DEFAULT_X86
	&platform_default_x86,
#endif
	NULL
};
