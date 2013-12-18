/*
 * Copyright 2011, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
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

#include <inttypes.h>

#include "mosys/globals.h"	/* for verbosity */
#include "mosys/log.h"
#include "mosys/platform.h"

#include "intf/io.h"

#include "lib/math.h"
#include "lib/sensors.h"

#include "drivers/superio.h"
#include "drivers/ite/it8772.h"

static int in_sio_cfgmode;

static uint16_t ec_idx;
static uint16_t ec_data;

int it8772_enter(struct platform_intf *intf, uint16_t *port)
{
	if (in_sio_cfgmode)
		return 0;

	/*
	 * There are two sequences for entering config mode:
	 * 0x2e/0x2f: 0x87 0x01 0x55 0x55
	 * 0x4e/0x4f: 0x87 0x01 0x55 0xaa
	 */
	switch(*port) {
	case 0x2e:
		io_write8(intf, *port, 0x87);
		io_write8(intf, *port, 0x01);
		io_write8(intf, *port, 0x55);
		io_write8(intf, *port, 0x55);
		break;
	case 0x4e:
		io_write8(intf, *port, 0x87);
		io_write8(intf, *port, 0x01);
		io_write8(intf, *port, 0x55);
		io_write8(intf, *port, 0xaa);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: Invalid port: 0x%04x\n",
		                   __func__, *port);
		return -1;
	}

	in_sio_cfgmode = 1;
	return 0;
}

int it8772_exit(struct platform_intf *intf, uint16_t *port)
{
	uint8_t cfg_ctl;

	if ((*port != 0x2e) && (*port != 0x4e)) {
		lprintf(LOG_DEBUG, "%s: Invalid port: 0x%04x\n",
		                   __func__, *port);
		return -1;
	}

	if (!in_sio_cfgmode)
		return 0;

	/* set bit 2 of the (global) config control register */
	cfg_ctl = sio_read(intf, *port, IT8772_CFG_CTL);
	cfg_ctl |= 2;
	sio_write(intf, *port, IT8772_CFG_CTL, cfg_ctl);

	in_sio_cfgmode = 0;
	return 0;
}

/*
 * returns 1 to indicate success
 * returns 0 if no port determined, but no error occurred
 * returns <0 to indicate error
 */
int it8772_get_sioport(struct platform_intf *intf, uint16_t *port)
{
	int rc = 0, i;
	static int port_internal = -1;
	uint16_t ports[] = { 0x2e, 0x4e };

	if (port_internal >= 0) {
		*port = (uint16_t)port_internal;
		return 1;
	}

	for (i = 0; i < ARRAY_SIZE(ports); i++) {
		uint16_t tmp8;

		it8772_enter(intf, &ports[i]);

		/* read a known byte to confirm we're in config mode */
		tmp8 = sio_read(intf, ports[i], SIO_CHIPID1);
		if (tmp8 == 0x87) {
			port_internal = ports[i];
			break;
		}
	}

	if (port_internal < 0) {
		lprintf(LOG_DEBUG, "%s: Port probing failed\n", __func__);
		rc = 0;
	} else {
		lprintf(LOG_DEBUG, "%s: Using port 0x%02x\n",
		                   __func__, port_internal);
		*port = (uint16_t)port_internal;
		rc = 1;
	}

	return rc;
}

/*
 * returns 1 to indicate it8772 found
 * returns 0 otherwise
 */
int it8772_detect(struct platform_intf *intf)
{
	uint16_t chipid = 0;
	uint16_t port;

	if (it8772_get_sioport(intf, &port) <= 0)
		return 0;

	chipid = sio_read(intf, port, SIO_CHIPID1) << 8 |
	         sio_read(intf, port, SIO_CHIPID2);

	lprintf(LOG_INFO, "chipid: 0x%04x\n", chipid);
	switch(chipid){
	case 0x8772:
		lprintf(LOG_DEBUG, "%s: found it8772\n", __func__);
		break;
	default:
		lprintf(LOG_DEBUG, "%s: failed to detect it8772\n", __func__);
		return 0;
	}

	it8772_exit(intf, &port);
	return 1;
}

static int it8772_setup_iobars(struct platform_intf *intf)
{
	uint16_t iobar, port = 0;

	if (it8772_get_sioport(intf, &port) <= 0)
		return -1;

	if (it8772_enter(intf, &port) < 0)
		return -1;

	/* Environment Controller */
	sio_write(intf, port, SIO_LDNSEL, IT8772_LDN_EC);
	iobar = (sio_read(intf, port, IT8772_EC_IOBAR_MSB) << 8) |
	         sio_read(intf, port, IT8772_EC_IOBAR_LSB);
	ec_idx = iobar + 0x05;
	ec_data = ec_idx + 1;
	lprintf(LOG_DEBUG, "%s: ec iobar: 0x%04x, ec_idx: 0x%04x, ec_data: "
	                   "0x%04x\n", __func__, iobar, ec_idx, ec_data);

	it8772_exit(intf, &port);
	return 0;
}

int it8772_read_voltage(struct platform_intf *intf,
                        struct sensor *sensor,
			struct sensor_reading *reading)
{
	uint8_t tmp8;
	struct it8772_priv *priv = sensor->priv;

	if (!ec_idx) {
		if (it8772_setup_iobars(intf) < 0)
			return -1;
	}

	if (io_write8(intf, ec_idx, sensor->addr.reg) < 0)
		return -1;
	if (io_read8(intf, ec_data, &tmp8) < 0)
		return -1;

	if (priv)
		reading->value = (double)tmp8 * priv->voltage_scaler;
	else
		reading->value = (double)tmp8;

	lprintf(LOG_DEBUG, "%s: reg: 0x%02x, raw: 0x%02x, reading: %f\n",
	                   __func__, sensor->addr.reg, tmp8, reading->value);
	return 0;
}

static int it8772_read_fantach_tach(struct platform_intf *intf,
                                    struct sensor *sensor, double *rpm)
{
	struct it8772_priv *priv = sensor->priv;
	uint16_t raw;
	uint8_t tmp8;
	uint8_t fan_poles;

	if (priv)
		fan_poles = priv->fan_poles;
	else
		fan_poles = 2;

	/* fan tach reading */
	if (io_write8(intf, ec_idx, sensor->addr.reg) < 0)
		return -1;
	if (io_read8(intf, ec_data, &tmp8) < 0)
		return -1;
	raw = tmp8;

	/* fan tach extended reading */
	if (io_write8(intf, ec_idx, sensor->addr.reg + 0x0b) < 0)
		return -1;
	if (io_read8(intf, ec_data, &tmp8) < 0)
		return -1;
	raw |= tmp8 << 8;

	/* RPM = (22.5KHz * 60sec) / (Count * Divisor), divisor = poles */
	if ((raw == 0xffff) || (raw == 0))
		*rpm = 0.0;
	else
		*rpm = (22500 * 60) / (raw * fan_poles);

	lprintf(LOG_DEBUG, "%s: reg: 0x%02x, raw: 0x%04x, "
	                   "fan_poles: %u, value: %f\n",
	                   __func__, sensor->addr.reg, raw,
			   fan_poles, *rpm);
	return 0;
}

static int it8772_read_fantach_pwm(struct platform_intf *intf,
                                   struct sensor *sensor, double *rpm)
{
	uint8_t tmp8;
	uint8_t pwm_reg;

	switch(sensor->addr.reg) {
	case IT8772_EC_FANTACH2_READING:
		pwm_reg = IT8772_EC_FANCTL2_PWM_VAL;
		break;
	case IT8772_EC_FANTACH3_READING:
		pwm_reg = IT8772_EC_FANCTL3_PWM_VAL;
		break;
	default:
		return -1;
	}

	/* fan tach reading */
	if (io_write8(intf, ec_idx, pwm_reg) < 0)
		return -1;
	if (io_read8(intf, ec_data, &tmp8) < 0)
		return -1;

	/* In any automatic mode, RPM =~ 16 * PWM value */
	*rpm = tmp8 * 16;

	return 0;
}

int it8772_read_fantach(struct platform_intf *intf,
                        struct sensor *sensor,
                        struct sensor_reading *reading)
{
	struct it8772_priv *priv = sensor->priv;
	uint8_t tmp8, force_read_tach, auto_mode_mask;
	int rc = 0;

	if (priv)
		force_read_tach = priv->force_read_tach;
	else
		force_read_tach = 0;

	if (!ec_idx) {
		if (it8772_setup_iobars(intf) < 0)
			return -1;
	}

	if (mosys_get_verbosity() >= LOG_DEBUG) {
		io_write8(intf, ec_idx, IT8772_EC_FANCTL_MAIN);
		io_read8(intf, ec_data, &tmp8);
		lprintf(LOG_DEBUG, "FAN_CTL main control register: 0x%02x "
		                   "(Default: 0x07)\n", tmp8);
		lprintf(LOG_DEBUG, "\tFAN_TAC2 Enabled: %s, FAN_TAC3 Enabled: "
		                   "%s\n", tmp8 & 6 ? "yes" : "no",
		                   tmp8 & 5 ? "yes" : "no");
		lprintf(LOG_DEBUG, "\tforce_read_tach Enabled: %s\n",
		                   force_read_tach ? "yes" : "no");

		io_write8(intf, ec_idx, IT8772_EC_FANCTL);
		io_read8(intf, ec_data, &tmp8);
		lprintf(LOG_DEBUG, "FAN_CTL control register: 0x%02x\n", tmp8);

		io_write8(intf, ec_idx, IT8772_EC_FANCTL2_PWM_CTL);
		io_read8(intf, ec_data, &tmp8);
		lprintf(LOG_DEBUG, "FAN_CTL 2 PWM: 0x%02x\n", tmp8);

		io_write8(intf, ec_idx, IT8772_EC_FANCTL3_PWM_CTL);
		io_read8(intf, ec_data, &tmp8);
		lprintf(LOG_DEBUG, "FAN_CTL 3 PWM: 0x%02x\n", tmp8);
	}

	/*
	 * Fan tach is read two different ways, depending on if output mode
	 * is set for manual or automatic operation.
	 */
	io_write8(intf, ec_idx, IT8772_EC_FANCTL_MAIN);
	io_read8(intf, ec_data, &tmp8);
	switch(sensor->addr.reg) {
	case IT8772_EC_FANTACH2_READING:
		auto_mode_mask = 1 << 1;
		break;
	case IT8772_EC_FANTACH3_READING:
		auto_mode_mask = 1 << 2;
		break;
	default:
		return -1;
	}
	if (!force_read_tach && (tmp8 & auto_mode_mask))
		rc = it8772_read_fantach_pwm(intf, sensor, &reading->value);
	else
		rc = it8772_read_fantach_tach(intf, sensor, &reading->value);

	io_write8(intf, ec_idx, IT8772_EC_INTERFACE);
	io_read8(intf, ec_data, &tmp8);
	lprintf(LOG_DEBUG, "Fan interface: 0x%02x\n", tmp8);
	if (tmp8 & 0x30)
		reading->mode = SENSOR_MODE_AUTO;
	else
		reading->mode = SENSOR_MODE_MANUAL;

	return rc;
}

int it8772_set_fan_control_mode(struct platform_intf *intf,
                                unsigned int fan_num,
                                enum it8772_fan_control_mode fan_mode)
{
	uint8_t auto_ctl, pwm_ctl;
	uint8_t tmp8;

	lprintf(LOG_DEBUG, "Setting fan %d to control mode %d\n",
	        fan_num, fan_mode);
	if (!ec_idx) {
		if (it8772_setup_iobars(intf) < 0)
			return -1;
	}

	switch(fan_num) {
	case 2:
		auto_ctl = IT8772_EC_FANCTL2_AUTO_CTL;
		pwm_ctl = IT8772_EC_FANCTL2_PWM_CTL;
		break;
	case 3:
		auto_ctl = IT8772_EC_FANCTL3_AUTO_CTL;
		pwm_ctl = IT8772_EC_FANCTL3_PWM_CTL;
		break;
	default:
		return -1;
	}

	switch(fan_mode) {
	case IT8772_FAN_CONTROL_PWM_SOFTWARE:
		/* set PWM controls to "software operation" */
		io_write8(intf, ec_idx, pwm_ctl);
		io_read8(intf, ec_data, &tmp8);
		tmp8 &= ~(1 << 7);
		io_write8(intf, ec_data, tmp8);

		/* disable smoothing (make fan react to changes immediately) */
		io_write8(intf, ec_idx, auto_ctl);
		io_read8(intf, ec_data, &tmp8);
		tmp8 &= ~(1 << 7);
		io_write8(intf, ec_data, tmp8);
		break;
	case IT8772_FAN_CONTROL_PWM_AUTOMATIC:
		/* set PWM controls to "automatic operation" */
		io_write8(intf, ec_idx, pwm_ctl);
		io_read8(intf, ec_data, &tmp8);
		tmp8 |= 1 << 7;
		io_write8(intf, ec_data, tmp8);

		/* enable smoothing */
		io_write8(intf, ec_idx, auto_ctl);
		io_read8(intf, ec_data, &tmp8);
		tmp8 |= 1 << 7;
		io_write8(intf, ec_data, tmp8);
		break;
	default:
		return -1;
	}

	return 0;
}

int it8772_set_pwm(struct platform_intf *intf,
                   unsigned int fan_num, unsigned int percent)
{
	uint8_t tmp8;
	uint8_t pwm_reg;

	if (!ec_idx) {
		if (it8772_setup_iobars(intf) < 0)
			return -1;
	}

	switch(fan_num) {
	case 2:
		pwm_reg = IT8772_EC_FANCTL2_PWM_VAL;
		break;
	case 3:
		pwm_reg = IT8772_EC_FANCTL3_PWM_VAL;
		break;
	default:
		return -1;
	}

	/* disable external interfaces (e.g. PECI) */
	io_write8(intf, ec_idx, IT8772_EC_INTERFACE);
	io_read8(intf, ec_data, &tmp8);
	tmp8 &= 0xcf;
	io_write8(intf, ec_data, tmp8);

	/* set FAN_CTL3-2 output mode to SmartGuardian */
	io_write8(intf, ec_idx, IT8772_EC_FANCTL_MAIN);
	io_read8(intf, ec_data, &tmp8);
	if (fan_num == 2)
		tmp8 |= 1 << 1;
	else if (fan_num == 3)
		tmp8 |= 1 << 2;
	io_write8(intf, ec_data, tmp8);

	/* set PWM controls to "software operation" */
	it8772_set_fan_control_mode(intf, fan_num,
	                            IT8772_FAN_CONTROL_PWM_SOFTWARE);

	/* set PWM (256 steps) */
	tmp8 = (percent * 255) / 100;
	lprintf(LOG_DEBUG, "%s: Setting fan%u PWM to %u/0x%02x (percent/raw)\n",
	                   __func__, fan_num, percent, tmp8);
	io_write8(intf, ec_idx, pwm_reg);
	io_write8(intf, ec_data, tmp8);

	return 0;
}

int it8772_set_fan_peci(struct platform_intf *intf, unsigned int fan_num)
{
	uint8_t tmp8;

	lprintf(LOG_DEBUG, "Setting fan %d to PECI mode\n", fan_num);
	if (!ec_idx) {
		if (it8772_setup_iobars(intf) < 0)
			return -1;
	}

	/* set external interface to PECI */
	io_write8(intf, ec_idx, IT8772_EC_INTERFACE);
	io_read8(intf, ec_data, &tmp8);
	tmp8 &= ~(0x30);
	tmp8 |= 0x20;
	io_write8(intf, ec_data, tmp8);

	/* Enable "SmartGuardian" mode */
	io_write8(intf, ec_idx, IT8772_EC_FANCTL_MAIN);
	io_read8(intf, ec_data, &tmp8);
	if (fan_num == 3)
		tmp8 |= 1 << 2;
	else if (fan_num == 2)
		tmp8 |= 1 << 1;
	io_write8(intf, ec_data, tmp8);

	return 0;
}

int it8772_disable_fan(struct platform_intf *intf, unsigned int fan_num)
{
	uint8_t tmp8;

	if (!ec_idx) {
		if (it8772_setup_iobars(intf) < 0)
			return -1;
	}

	/* disable external interfaces (e.g. PECI) */
	io_write8(intf, ec_idx, IT8772_EC_INTERFACE);
	io_read8(intf, ec_data, &tmp8);
	tmp8 &= 0xcf;
	io_write8(intf, ec_data, tmp8);

	/* set FAN_CTL3-2 output mode to ON/OFF */
	io_write8(intf, ec_idx, IT8772_EC_FANCTL_MAIN);
	io_read8(intf, ec_data, &tmp8);
	if (fan_num == 2)
		tmp8 &= ~(1 << 1);
	else if (fan_num == 3)
		tmp8 &= ~(1 << 2);
	io_write8(intf, ec_data, tmp8);

	/* now to actually turn the fan ON or OFF */
	io_write8(intf, ec_idx, IT8772_EC_FANCTL);
	io_read8(intf, ec_data, &tmp8);
	if (fan_num == 2)
		tmp8 &= ~(1 << 1);
	else if (fan_num == 3)
		tmp8 &= ~(1 << 2);
	io_write8(intf, ec_data, tmp8);

	return 0;
}
