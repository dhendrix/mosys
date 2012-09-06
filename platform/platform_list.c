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

#include <stdlib.h>

#include "mosys/platform.h"

/* default */
extern struct platform_intf platform_default_x86;

/* experimental */
extern struct platform_intf platform_aebl_tegra2;
extern struct platform_intf platform_asymptote_tegra2;
extern struct platform_intf platform_butterfly;
extern struct platform_intf platform_daisy;
extern struct platform_intf platform_kaen_tegra2;
extern struct platform_intf platform_kiev;
extern struct platform_intf platform_link;
extern struct platform_intf platform_lumpy;
extern struct platform_intf platform_parrot;
extern struct platform_intf platform_seaboard_tegra2;
extern struct platform_intf platform_stumpy;

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
#ifdef CONFIG_EXPERIMENTAL_ASYMPTOTE
	&platform_asymptote_tegra2,
#endif
#ifdef CONFIG_EXPERIMENTAL_BUTTERFLY
	&platform_butterfly,
#endif
#ifdef CONFIG_EXPERIMENTAL_DAISY
	&platform_daisy,
#endif
#ifdef CONFIG_EXPERIMENTAL_KAEN
	&platform_kaen_tegra2,
#endif
#ifdef CONFIG_EXPERIMENTAL_KIEV
	&platform_kiev,
#endif
#ifdef CONFIG_EXPERIMENTAL_LINK
	&platform_link,
#endif
#ifdef CONFIG_EXPERIMENTAL_LUMPY
	&platform_lumpy,
#endif
#ifdef CONFIG_EXPERIMENTAL_PARROT
	&platform_parrot,
#endif
#ifdef CONFIG_EXPERIMENTAL_SEABOARD
	&platform_seaboard_tegra2,
#endif
#ifdef CONFIG_EXPERIMENTAL_STUMPY
	&platform_stumpy,
#endif

/* place default platform last */
#ifdef CONFIG_DEFAULT_X86
	&platform_default_x86,
#endif
	NULL
};
