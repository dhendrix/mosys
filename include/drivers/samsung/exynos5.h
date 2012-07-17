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

#ifndef MOSYS_DRIVERS_SAMSUNG_EXYNOS5_H__
#define MOSYS_DRIVERS_SAMSUNG_EXYNOS5_H__

#include "drivers/samsung/exynos_generic.h"	/* for exynos_gpio_bank */

enum exynos5_gpio_port {
	/* Note: use same ordering as exynos5_gpio_banks */
	EXYNOS5_GPA0,
	EXYNOS5_GPA1,
	EXYNOS5_GPA2,

	EXYNOS5_GPB0,
	EXYNOS5_GPB1,
	EXYNOS5_GPB2,
	EXYNOS5_GPB3,

	EXYNOS5_GPC0,
	EXYNOS5_GPC1,
	EXYNOS5_GPC2,
	EXYNOS5_GPC3,

	EXYNOS5_GPD0,
	EXYNOS5_GPD1,

	EXYNOS5_GPY0,
	EXYNOS5_GPY1,
	EXYNOS5_GPY2,
	EXYNOS5_GPY3,
	EXYNOS5_GPY4,
	EXYNOS5_GPY5,
	EXYNOS5_GPY6,

	EXYNOS5_GPX0,
	EXYNOS5_GPX1,
	EXYNOS5_GPX2,
	EXYNOS5_GPX3,

	EXYNOS5_GPE0,
	EXYNOS5_GPE1,

	EXYNOS5_GPF0,
	EXYNOS5_GPF1,

	EXYNOS5_GPG0,
	EXYNOS5_GPG1,
	EXYNOS5_GPG2,

	EXYNOS5_GPH0,
	EXYNOS5_GPH1,

	EXYNOS5_GPV0,
	EXYNOS5_GPV1,
	EXYNOS5_GPV2,
	EXYNOS5_GPV3,
	EXYNOS5_GPV4,

	EXYNOS5_GPZ,
};

extern const struct exynos_gpio_bank exynos5_gpio_banks[];
extern int exynos5_read_gpio(struct platform_intf *intf, struct gpio_map *gpio);
extern int exynos5_read_gpio_mvl(struct platform_intf *intf,
				 struct gpio_map *gpio);
extern int exynos5_set_gpio(struct platform_intf *intf,
			    struct gpio_map *gpio, int state);
extern int exynos5_gpio_list(struct platform_intf *intf);

#endif /* MOSYS_DRIVERS_SAMSUNG_EXYNOS5_H__ */
