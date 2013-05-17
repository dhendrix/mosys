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

#ifndef MOSYS_DRIVERS_SAMSUNG_EXYNOS5250_H__
#define MOSYS_DRIVERS_SAMSUNG_EXYNOS5250_H__

#include "drivers/samsung/exynos_generic.h"	/* for exynos_gpio_bank */

enum exynos5250_gpio_port {
	/* Note: use same ordering as exynos5250_gpio_banks */
	EXYNOS5250_GPA0,
	EXYNOS5250_GPA1,
	EXYNOS5250_GPA2,

	EXYNOS5250_GPB0,
	EXYNOS5250_GPB1,
	EXYNOS5250_GPB2,
	EXYNOS5250_GPB3,

	EXYNOS5250_GPC0,
	EXYNOS5250_GPC1,
	EXYNOS5250_GPC2,
	EXYNOS5250_GPC3,

	EXYNOS5250_GPD0,
	EXYNOS5250_GPD1,

	EXYNOS5250_GPY0,
	EXYNOS5250_GPY1,
	EXYNOS5250_GPY2,
	EXYNOS5250_GPY3,
	EXYNOS5250_GPY4,
	EXYNOS5250_GPY5,
	EXYNOS5250_GPY6,

	EXYNOS5250_GPX0,
	EXYNOS5250_GPX1,
	EXYNOS5250_GPX2,
	EXYNOS5250_GPX3,

	EXYNOS5250_GPE0,
	EXYNOS5250_GPE1,

	EXYNOS5250_GPF0,
	EXYNOS5250_GPF1,

	EXYNOS5250_GPG0,
	EXYNOS5250_GPG1,
	EXYNOS5250_GPG2,

	EXYNOS5250_GPH0,
	EXYNOS5250_GPH1,

	EXYNOS5250_GPV0,
	EXYNOS5250_GPV1,
	EXYNOS5250_GPV2,
	EXYNOS5250_GPV3,
	EXYNOS5250_GPV4,

	EXYNOS5250_GPZ,
};

extern const struct exynos_gpio_bank exynos5250_gpio_banks[];
extern int exynos5250_read_gpio(struct platform_intf *intf,
				struct gpio_map *gpio);
extern int exynos5250_read_gpio_mvl(struct platform_intf *intf,
				 struct gpio_map *gpio);
extern int exynos5250_set_gpio(struct platform_intf *intf,
			    struct gpio_map *gpio, int state);
extern int exynos5250_gpio_list(struct platform_intf *intf);

#endif /* MOSYS_DRIVERS_SAMSUNG_EXYNOS5250_H__ */
