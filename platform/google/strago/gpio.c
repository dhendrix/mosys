/*
 * Copyright 2014, Google Inc.
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

#include "strago.h"
#include "drivers/gpio.h"
#include "drivers/intel/braswell.h"
#include "mosys/platform.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"

static struct gpio_map strago_platform_gpio_map[] = {
/* id(Family+Pad no), type, dev, port, pin, neg, devname, gpio name */
	/*------------------- North community -------------------------*/
	{0,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX0"},
	{1,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX3"},
	{2,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX7"},
	{3,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX1"},
	{4,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX5"},
	{5,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX4"},
	{6,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX8"},
	{7,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX2"},
	{8,  GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_DFX6"},
	{15, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS0"},
	{16, GPIO_OUT, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SEC_GPIO_SUS10"},
	{17, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS3"},
	{18, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS7"},
	{19, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS1"},
	{20, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS5"},
	{21, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SEC_GPIO_SUS11"},
	{22, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS4"},
	{23, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SEC_GPIO_SUS8"},
	{24, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS2"},
	{25, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GPIO_SUS6"},
	{26, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "CX_PREQ_B"},
	{27, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SEC_GPIO_SUS9"},
	{30, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "TRST_B"},
	{31, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "TCK"},
	{32, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PROCHOT_B"},
	{33, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SVID0_DATA"},
	{34, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "TMS"},
	{35, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "CX_PRDY_B_2"},
	{36, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "TDO_2"},
	{37, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "CX_PRDY_B"},
	{38, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SVID0_ALERT_B"},
	{39, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "TDO"},
	{40, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "SVID0_CLK"},
	{41, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "TDI"},
	{45, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB05"},
	{46, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB02"},
	{47, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB08"},
	{48, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB00"},
	{49, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASBO6"},
	{50, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB10"},
	{51, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB03"},
	{52, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB01"},
	{53, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB07"},
	{54, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB11"},
	{55, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "GP_CAMERASB04"},
	{56, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PANEL0_BKLTEN"},
	{60, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI0_HPD"},
	{61, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI2_DDC_SDA"},
	{62, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PANEL1_BKLTCTL"},
	{63, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI1_HPD"},
	{64, GPIO_OUT, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI1_HPD"},
	{65, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PANEL0_BKLTCTL"},
	{66, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI0_DDC_SDA"},
	{67, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI2_DDC_SCL"},
	{68, GPIO_OUT, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI2_HPD"},
	{69, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PANEL1_VDDEN"},
	{70, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PANEL1_BKLTEN"},
	{71, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "HV_DDI0_DDC_SDL"},
	{72, GPIO_IN, 0, BSW_GPNCORE_PORT, 0, 0, NULL, "PANEL0_VDDEN"},
	/*------------------- South east community ----------------------*/
	{0, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_PLT_CLK0"},
	{1, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "PWM1"},
	{2, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_PLT_CLK1"},
	{3, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_PLT_CLK4"},
	{4, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_PLT_CLK3"},
	{5, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "PWM0"},
	{6, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_PLT_CLK5"},
	{7, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_PLT_CLK2"},
	{15, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC2_D3_CD_B"},
	{16, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC1_CLK"},
	{17, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC1_D0"},
	{18, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC2_D1"},
	{19, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC2_CLK"},
	{20, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC1_D2"},
	{21, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC2_D2"},
	{22, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC2_CMD"},
	{23, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC1_CMD"},
	{24, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC1_D1"},
	{25, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC2_D0"},
	{26, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC1_D3_CD_B"},
	{30, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_D1"},
	{31, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_CLK"},
	{32, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_D3"},
	{33, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_D2"},
	{34, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_CMD"},
	{35, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_D0"},
	{45, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_LPC_AD2"},
	{46, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "LPC_CLKRUNB"},
	{47, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_LPC_AD0"},
	{48, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "LPC_FRAMEB"},
	{49, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_LPC_CLKOUT1"},
	{50, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_LPC_AD3"},
	{51, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_LPC_CLKOUT0"},
	{52, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MF_LPC_AD1"},
	{60, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPI1_MISO"},
	{61, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPI1_CS0_B"},
	{62, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPI1_CLK"},
	{63, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MMC1_D6"},
	{64, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPI1_MOSI"},
	{65, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MMC1_D5"},
	{66, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPI1_CS1_B"},
	{67, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MMC1_D4_SD_WE"},
	{68, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MMC1_D7"},
	{69, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "MMC1_RCLK"},
	{75, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "USB_OC1_B"},
	{76, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "PMU_RESETBUTTON_B"},
	{77, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "GPIO_ALERT"},
	{78, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_PWR_EN_B"},
	{79, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "ILB_SERIRQ"},
	{80, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "USB_OC0_B"},
	{81, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_CD_B"},
	{82, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPKR"},
	{83, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SUSPWRDNACK"},
	{84, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SPARE_PIN"},
	{85, GPIO_IN, 0, BSW_GPSECORE_PORT, 0, 0, NULL, "SDMMC3_1P8_EN"},
	/*-------------- South west community ------------------*/
	{0, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_D2"},
	{1, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_D0"},
	{2, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_CLK"},
	{3, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_D3"},
	{4, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_CS1_B"},
	{5, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_D1"},
	{6, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_CS0_B"},
	{7, GPIO_OUT, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "FST_SPI_CS2_B"},
	{15, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART1_RTS_B"},
	{16, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART1_RXD"},
	{17, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART2_RXD"},
	{18, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART1_CTS_B"},
	{19, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART2_RTS_B"},
	{20, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART1_TXD"},
	{21, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART2_TXD"},
	{22, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "UART2_CTS_B"},
	{30, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_CLK"},
	{31, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_RSTB"},
	{32, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_SDIO"},
	{33, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_SDO"},
	{34, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_DOCKRSTB"},
	{35, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_SYNC"},
	{36, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_SDI1"},
	{37, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_HDA_DOCKENB"},
	{45, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C5_SDA"},
	{46, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C4_SDA"},
	{47, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C6_SDA"},
	{48, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C5_SCL"},
	{49, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C_NFC_SDA"},
	{50, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C4_SCL"},
	{51, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C6_SCL"},
	{52, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C_NFC_SCL"},
	{60, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C1_SDA"},
	{61, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C0_SDA"},
	{62, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C2_SDA"},
	{63, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C1_SCL"},
	{64, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C3_SDA"},
	{65, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C0_SCL"},
	{66, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C2_SCL"},
	{67, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "I2C3_SCL"},
	{75, GPIO_OUT, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "SATA_GP0"},
	{76, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "SATA_GP1"},
	{77, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "SATA_LEDN"},
	{78, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "SATA_GP2"},
	{79, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_SMB_ALERTB"},
	{80, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "SATA_GP3"},
	{81, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_SMB_CLK"},
	{82, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "MF_SMB_DATA"},
	{90, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "PCIE_CLKREQ0B"},
	{91, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "PCIE_CLKREQ1B"},
	{92, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "GP_SSP_2_CLK"},
	{93, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "PCIE_CLKREQ2B"},
	{94, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "GP_SSP_2_RXD"},
	{95, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "PCIE_CLKREQ3B"},
	{96, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "GP_SSP_2_FS"},
	{97, GPIO_IN, 0, BSW_GPSWCORE_PORT, 0, 0, NULL, "GP_SSP_2_TXD"},
	/*---------------------- East community ---------------------*/
	{0, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_SLP_S3_B"},
	{1, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_BATLOW_B"},
	{2, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "SUS_STAT_B"},
	{3, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_SLP_S0IX_B"},
	{4, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_AC_PRESENT"},
	{5, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_PLTRST_B"},
	{6, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_SUSCLK"},
	{7, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_SLP_LAN_B"},
	{8, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_PWRBTN_B"},
	{9, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_SLP_S4_B"},
	{10, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_WAKE_B"},
	{11, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "PMU_WAKE_LAN_B"},
	{15, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_3"},
	{16, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_7"},
	{17, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_I2C1_SCL"},
	{18, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_1"},
	{19, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_5"},
	{20, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_9"},
	{21, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_0"},
	{22, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_4"},
	{23, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_8"},
	{24, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_2"},
	{25, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_GPIO_6"},
	{26, GPIO_IN, 0, BSW_GPECORE_PORT, 0, 0, NULL, "MF_ISH_I2C1_SDA"},
	{0, 0,       0, 0,                     0, 0, NULL,  NULL      },
};

/*
 * strago_gpio_read  -  read level for a specific GPIO
 *
 * @intf:       platform interface
 * @name:       name of GPIO to get state for
 * @gpio:       gpio id, name, port etc
 *
 * returns GPIO level (0 or 1) to indicate success
 * returns <0 to indicate failure
 */
static int strago_gpio_read(struct platform_intf *intf, struct gpio_map *gpio)
{
	return braswell_read_gpio(intf, gpio);
}

/* get mapping for a specific GPIO */
static struct gpio_map *strago_gpio_map(struct platform_intf *intf,
						const char *name)
{
	struct gpio_map *gpio;

	for (gpio = strago_platform_gpio_map; gpio->name; gpio++) {
		if (!strcmp(gpio->name, name))
			return gpio;
	}
	return NULL;
}

static int strago_kv_pair_print_gpio(struct gpio_map *gpio, int state)
{
	struct kv_pair *kv;
	int rc;
	static const char * const bank_name[] = {"N", "SE", "SW", "E"};

	kv = kv_pair_new();
	kv_pair_add(kv, "device", gpio->devname);
	kv_pair_fmt(kv, "id", "%s%02u", bank_name[gpio->port], gpio->id);

	switch (gpio->type) {
	case GPIO_IN:
		kv_pair_add(kv, "type", "IN");
		break;
	case GPIO_OUT:
		kv_pair_add(kv, "type", "OUT");
		break;
	case GPIO_ALT:
		kv_pair_add(kv, "type", "HIZ");
		break;
	default:
		kv_pair_free(kv);
		return -1;
	}

	kv_pair_fmt(kv, "state", "%d", state);
	kv_pair_add(kv, "name", gpio->name);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);
	return rc;
}

/* list all GPIOs for the platform */
static int strago_gpio_list(struct platform_intf *intf)
{
	int i = 0, state = 0;
	struct gpio_map gpio;

	for (i = 0; strago_platform_gpio_map[i].name != NULL; i++) {
		gpio = strago_platform_gpio_map[i];
		state = strago_gpio_read(intf, &gpio);
		strago_kv_pair_print_gpio(&gpio, state);
	}
	return 0;
}

/* set a specific GPIO state to 0 or 1 */
static int strago_gpio_set(struct platform_intf *intf, const char *name,
								int state)
{
	struct gpio_map *gpio;

	gpio = strago_gpio_map(intf, name);
	if (!gpio) {
		lprintf(LOG_ERR, "Error: gpio %s not found\n", name);
		return -1;
	}

	braswell_set_gpio(intf, gpio, state);
	return 0;
}

struct gpio_cb strago_gpio_cb = {
	.read	= strago_gpio_read,
	.map    = strago_gpio_map,
	.list   = strago_gpio_list,
	.set    = strago_gpio_set
};
