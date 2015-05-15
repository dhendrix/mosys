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

/* experimental */
extern struct platform_intf platform_kiev;
extern struct platform_intf platform_storm;
extern struct platform_intf platform_glados;
extern struct platform_intf platform_strago;

/* production platforms */
extern struct platform_intf platform_alex;
extern struct platform_intf platform_auron;
extern struct platform_intf platform_beltino;
extern struct platform_intf platform_butterfly;
extern struct platform_intf platform_daisy;
extern struct platform_intf platform_link;
extern struct platform_intf platform_lumpy;
extern struct platform_intf platform_mario;
extern struct platform_intf platform_nyan;
extern struct platform_intf platform_oak;
extern struct platform_intf platform_parrot;
extern struct platform_intf platform_peach;
extern struct platform_intf platform_pinky;
extern struct platform_intf platform_rambi;
extern struct platform_intf platform_rush;
extern struct platform_intf platform_samus;
extern struct platform_intf platform_skate;
extern struct platform_intf platform_slippy;
extern struct platform_intf platform_spring;
extern struct platform_intf platform_stout;
extern struct platform_intf platform_stumpy;
extern struct platform_intf platform_z600;
extern struct platform_intf platform_zgb;

struct platform_intf *platform_intf_list[] = {
#ifdef CONFIG_PLATFORM_ALEX
	&platform_alex,
#endif
#ifdef CONFIG_PLATFORM_AURON
	&platform_auron,
#endif
#ifdef CONFIG_PLATFORM_BELTINO
	&platform_beltino,
#endif
#ifdef CONFIG_PLATFORM_BUTTERFLY
	&platform_butterfly,
#endif
#ifdef CONFIG_PLATFORM_DAISY
	&platform_daisy,
#endif
#ifdef CONFIG_PLATFORM_LINK
	&platform_link,
#endif
#ifdef CONFIG_PLATFORM_LUMPY
	&platform_lumpy,
#endif
#ifdef CONFIG_PLATFORM_MARIO
	&platform_mario,
#endif
#ifdef CONFIG_PLATFORM_NYAN
	&platform_nyan,
#endif
#ifdef CONFIG_PLATFORM_OAK
	&platform_oak,
#endif
#ifdef CONFIG_PLATFORM_PARROT
	&platform_parrot,
#endif
#ifdef CONFIG_PLATFORM_PEACH
	&platform_peach,
#endif
#ifdef CONFIG_PLATFORM_PINKY
	&platform_pinky,
#endif
#ifdef CONFIG_PLATFORM_RAMBI
	&platform_rambi,
#endif
#ifdef CONFIG_PLATFORM_RUSH
	&platform_rush,
#endif
#ifdef CONFIG_PLATFORM_SAMUS
	&platform_samus,
#endif
#ifdef CONFIG_PLATFORM_SKATE
	&platform_skate,
#endif
#ifdef CONFIG_PLATFORM_SLIPPY
	&platform_slippy,
#endif
#ifdef CONFIG_PLATFORM_SPRING
	&platform_spring,
#endif
#ifdef CONFIG_PLATFORM_STOUT
	&platform_stout,
#endif
#ifdef CONFIG_PLATFORM_STUMPY
	&platform_stumpy,
#endif
#ifdef CONFIG_PLATFORM_Z600
	&platform_z600,
#endif
#ifdef CONFIG_PLATFORM_ZGB
	&platform_zgb,
#endif


/* experimental platforms */
#ifdef CONFIG_EXPERIMENTAL_KIEV
	&platform_kiev,
#endif
#ifdef CONFIG_EXPERIMENTAL_GLADOS
	&platform_glados,
#endif
#ifdef CONFIG_EXPERIMENTAL_STRAGO
	&platform_strago,
#endif
#ifdef CONFIG_PLATFORM_STORM
	&platform_storm,
#endif
	NULL
};
