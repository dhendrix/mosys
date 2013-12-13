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

#ifndef MOSYS_DRIVERS_SUPERIO_ITE_IT8772_H__
#define MOSYS_DRIVERS_SUPERIO_ITE_IT8772_H__

#include "lib/sensors.h"

#define IT8772_CFG_CTL			0x02	/* Global Configure Control */

#define IT8772_LDN_UART1		0x01	/* UART1 */
#define IT8772_LDN_EC			0x04	/* Environment Control */
#define IT8772_LDN_KEYBOARD		0x05	/* Keyboard */
#define IT8772_LDN_MOUSE		0x06	/* Mouse */
#define IT8772_LDN_GPIO			0x07	/* BRAM */

/* Base = (MSB << 8) + LSB, EC index = base + 0x05, EC data = base + 0x06 */
#define IT8772_EC_IOBAR_MSB		0x60
#define IT8772_EC_IOBAR_LSB		0x61

/* Environment Controller registers accessed thru EC index/data pair */
#define IT8772_EC_CFG			0x00

/* EC fan control registers */
#define IT8772_EC_INTERFACE		0x0a
#define IT8772_EC_FANTACH_CTL		0x0c
#define IT8772_EC_FANTACH2_READING	0x0e
#define IT8772_EC_FANTACH3_READING	0x0f
#define IT8772_EC_FANTACH2_LIMIT	0x11
#define IT8772_EC_FANTACH3_LIMIT	0x12
#define IT8772_EC_FANCTL_MAIN		0x13
#define IT8772_EC_FANCTL		0x14
#define IT8772_EC_FANCTL1_PWM_CTL	0x15
#define IT8772_EC_FANCTL2_PWM_CTL	0x16
#define IT8772_EC_FANCTL3_PWM_CTL	0x17
#define IT8772_EC_FANTACH2_EXT_READING	0x19
#define IT8772_EC_FANTACH3_EXT_READING	0x1a
#define IT8772_EC_FANTACH2_EXT_LIMIT	0x1c
#define IT8772_EC_FANTACH3_EXT_LIMIT	0x1d
#define IT8772_EC_FANCTL2_PWM_VAL	0x6b
#define IT8772_EC_FANCTL3_PWM_VAL	0x73
#define IT8772_EC_FANCTL2_AUTO_CTL	0x6c
#define IT8772_EC_FANCTL3_AUTO_CTL	0x74

/* EC voltage control registers */
#define IT8772_EC_VIN0_READING		0x20
#define IT8772_EC_VIN1_READING		0x21
#define IT8772_EC_VIN2_READING		0x22
#define IT8772_EC_VIN3_READING		0x23
#define IT8772_EC_VIN4_READING		0x24
#define IT8772_EC_VBAT_READING		0x25
#define IT8772_EC_VIN0_LIMIT_HIGH	0x30
#define IT8772_EC_VIN0_LIMIT_LOW	0x31
#define IT8772_EC_VIN1_LIMIT_HIGH	0x32
#define IT8772_EC_VIN1_LIMIT_LOW	0x33
#define IT8772_EC_VIN2_LIMIT_HIGH	0x34
#define IT8772_EC_VIN2_LIMIT_LOW	0x35
#define IT8772_EC_VIN3_LIMIT_HIGH	0x36
#define IT8772_EC_VIN3_LIMIT_LOW	0x37
#define IT8772_EC_VIN4_LIMIT_HIGH	0x38
#define IT8772_EC_VIN4_LIMIT_LOW	0x39
#define IT8772_EC_VIN7_LIMIT_HIGH	0x3e
#define IT8772_EC_VIN7_LIMIT_LOW	0x3f

/* EC temperature control registers */
#define IT8772_EC_TMPIN1_READING	0x29
#define IT8772_EC_TMPIN2_READING	0x2a
#define IT8772_EC_TMPIN3_READING	0x2b
#define IT8772_EC_TMPIN1_LIMIT_HIGH	0x40
#define IT8772_EC_TMPIN1_LIMIT_LOW	0x41
#define IT8772_EC_TMPIN2_LIMIT_HIGH	0x42
#define IT8772_EC_TMPIN2_LIMIT_LOW	0x43
#define IT8772_EC_TMPIN3_LIMIT_HIGH	0x44
#define IT8772_EC_TMPIN3_LIMIT_LOW	0x45

struct it8772_priv {
	uint8_t fan_poles;
	double voltage_scaler;
	uint8_t force_read_tach;
};

extern int it8772_enter(struct platform_intf *intf, uint16_t *port);
extern int it8772_exit(struct platform_intf *intf, uint16_t *port);

/*
 * it8772_get_sioport - return port used for super i/o config
 *
 * @intf:	platform interface
 * @port:	buffer to fill
 *
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
extern int it8772_get_sioport(struct platform_intf *intf, uint16_t *port);

/*
 * it8772_detect - detect it8772 super i/o
 *
 * @intf:	platform interface
 *
 * returns 1 to indicate mec1308 found
 * returns 0 if no super i/o found, but no error occurred
 * returns <0 to indicate error
 */
extern int it8772_detect(struct platform_intf *intf);


/*
 * it8772_read_fantach - read tachometer value
 *
 * @intf:	platform interface
 * @sensor:	sensor struct
 * @reading:	location to store reading
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int it8772_read_fantach(struct platform_intf *intf,
                               struct sensor *sensor,
                               struct sensor_reading *reading);

/*
 * it8772_read_voltage - read voltage value
 *
 * @intf:	platform interface
 * @sensor:	sensor struct
 * @reading:	location to store reading
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int it8772_read_voltage(struct platform_intf *intf,
                               struct sensor *sensor,
                               struct sensor_reading *reading);

/*
 * it8772_set_fan_pwm - set PWM for a given fan
 *
 * @intf:	platform interface
 * @sensor:	fan number
 * @percent:	percentage
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int it8772_set_pwm(struct platform_intf *intf,
                          unsigned int fan_num, unsigned int percent);

/*
 * it8772_set_fan_peci - set fan to use PECI interface
 *
 * @intf:	platform interface
 * @sensor:	fan number
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int it8772_set_fan_peci(struct platform_intf *intf,unsigned int fan_num);

/*
 * it8772_disable_fan - disable fan completely
 *
 * @intf:	platform interface
 * @sensor:	fan number
 *
 * returns 0 to indicate success
 * returns <0 to indicate error
 */
extern int it8772_disable_fan(struct platform_intf *intf, unsigned int fan_num);

#endif	/* MOSYS_DRIVERS_SUPERIO_ITE_IT8772_H__ */
