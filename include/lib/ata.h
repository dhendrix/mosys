/*
 *  Copyright 2003-2004 Red Hat, Inc.  All rights reserved.
 *  Copyright 2003-2004 Jeff Garzik
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  libata documentation is available via 'make {ps|pdf}docs',
 *  as Documentation/DocBook/libata.*
 *
 *  Hardware documentation available from http://www.t13.org/
 *
 */

#ifndef __ATA_H__
#define __ATA_H__

enum {
	/* various global constants */
	ATA_MAX_DEVICES		= 2,	/* per bus/port */
	ATA_MAX_PRD		= 256,	/* we could make these 256/256 */
	ATA_SECT_SIZE		= 512,
	ATA_MAX_SECTORS_128	= 128,
	ATA_MAX_SECTORS		= 256,
	ATA_MAX_SECTORS_LBA48	= 65535,/* TODO: 65536? */
	ATA_MAX_SECTORS_TAPE	= 65535,

	ATA_ID_WORDS		= 256,
	ATA_ID_CONFIG		= 0,
	ATA_ID_CYLS		= 1,
	ATA_ID_HEADS		= 3,
	ATA_ID_SECTORS		= 6,
	ATA_ID_SERNO		= 10,
	ATA_ID_BUF_SIZE		= 21,
	ATA_ID_FW_REV		= 23,
	ATA_ID_PROD		= 27,
	ATA_ID_MAX_MULTSECT	= 47,
	ATA_ID_DWORD_IO		= 48,
	ATA_ID_CAPABILITY	= 49,
	ATA_ID_OLD_PIO_MODES	= 51,
	ATA_ID_OLD_DMA_MODES	= 52,
	ATA_ID_FIELD_VALID	= 53,
	ATA_ID_CUR_CYLS		= 54,
	ATA_ID_CUR_HEADS	= 55,
	ATA_ID_CUR_SECTORS	= 56,
	ATA_ID_MULTSECT		= 59,
	ATA_ID_LBA_CAPACITY	= 60,
	ATA_ID_SWDMA_MODES	= 62,
	ATA_ID_MWDMA_MODES	= 63,
	ATA_ID_PIO_MODES	= 64,
	ATA_ID_EIDE_DMA_MIN	= 65,
	ATA_ID_EIDE_DMA_TIME	= 66,
	ATA_ID_EIDE_PIO		= 67,
	ATA_ID_EIDE_PIO_IORDY	= 68,
	ATA_ID_ADDITIONAL_SUPP	= 69,
	ATA_ID_QUEUE_DEPTH	= 75,
	ATA_ID_MAJOR_VER	= 80,
	ATA_ID_COMMAND_SET_1	= 82,
	ATA_ID_COMMAND_SET_2	= 83,
	ATA_ID_CFSSE		= 84,
	ATA_ID_CFS_ENABLE_1	= 85,
	ATA_ID_CFS_ENABLE_2	= 86,
	ATA_ID_CSF_DEFAULT	= 87,
	ATA_ID_UDMA_MODES	= 88,
	ATA_ID_HW_CONFIG	= 93,
	ATA_ID_SPG		= 98,
	ATA_ID_LBA_CAPACITY_2	= 100,
	ATA_ID_SECTOR_SIZE	= 106,
	ATA_ID_WWN		= 108,
	ATA_ID_LOGICAL_SECTOR_SIZE	= 117,	/* and 118 */
	ATA_ID_LAST_LUN		= 126,
	ATA_ID_DLF		= 128,
	ATA_ID_CSFO		= 129,
	ATA_ID_CFA_POWER	= 160,
	ATA_ID_CFA_KEY_MGMT	= 162,
	ATA_ID_CFA_MODES	= 163,
	ATA_ID_DATA_SET_MGMT	= 169,
	ATA_ID_ROT_SPEED	= 217,
	ATA_ID_PIO4		= (1 << 1),

	ATA_ID_SERNO_LEN	= 20,
	ATA_ID_FW_REV_LEN	= 8,
	ATA_ID_PROD_LEN		= 40,
	ATA_ID_WWN_LEN		= 8,

	ATA_PCI_CTL_OFS		= 2,

	ATA_PIO0		= (1 << 0),
	ATA_PIO1		= ATA_PIO0 | (1 << 1),
	ATA_PIO2		= ATA_PIO1 | (1 << 2),
	ATA_PIO3		= ATA_PIO2 | (1 << 3),
	ATA_PIO4		= ATA_PIO3 | (1 << 4),
	ATA_PIO5		= ATA_PIO4 | (1 << 5),
	ATA_PIO6		= ATA_PIO5 | (1 << 6),

	ATA_PIO4_ONLY		= (1 << 4),

	ATA_SWDMA0		= (1 << 0),
	ATA_SWDMA1		= ATA_SWDMA0 | (1 << 1),
	ATA_SWDMA2		= ATA_SWDMA1 | (1 << 2),

	ATA_SWDMA2_ONLY		= (1 << 2),

	ATA_MWDMA0		= (1 << 0),
	ATA_MWDMA1		= ATA_MWDMA0 | (1 << 1),
	ATA_MWDMA2		= ATA_MWDMA1 | (1 << 2),
	ATA_MWDMA3		= ATA_MWDMA2 | (1 << 3),
	ATA_MWDMA4		= ATA_MWDMA3 | (1 << 4),

	ATA_MWDMA12_ONLY	= (1 << 1) | (1 << 2),
	ATA_MWDMA2_ONLY		= (1 << 2),

	ATA_UDMA0		= (1 << 0),
	ATA_UDMA1		= ATA_UDMA0 | (1 << 1),
	ATA_UDMA2		= ATA_UDMA1 | (1 << 2),
	ATA_UDMA3		= ATA_UDMA2 | (1 << 3),
	ATA_UDMA4		= ATA_UDMA3 | (1 << 4),
	ATA_UDMA5		= ATA_UDMA4 | (1 << 5),
	ATA_UDMA6		= ATA_UDMA5 | (1 << 6),
	ATA_UDMA7		= ATA_UDMA6 | (1 << 7),
	/* ATA_UDMA7 is just for completeness... doesn't exist (yet?).  */

	ATA_UDMA24_ONLY		= (1 << 2) | (1 << 4),

	ATA_UDMA_MASK_40C	= ATA_UDMA2,	/* udma0-2 */

	/* DMA-related */
	ATA_PRD_SZ		= 8,
	ATA_PRD_TBL_SZ		= (ATA_MAX_PRD * ATA_PRD_SZ),
	ATA_PRD_EOT		= (1 << 31),	/* end-of-table flag */

	ATA_DMA_TABLE_OFS	= 4,
	ATA_DMA_STATUS		= 2,
	ATA_DMA_CMD		= 0,
	ATA_DMA_WR		= (1 << 3),
	ATA_DMA_START		= (1 << 0),
	ATA_DMA_INTR		= (1 << 2),
	ATA_DMA_ERR		= (1 << 1),
	ATA_DMA_ACTIVE		= (1 << 0),

	/* bits in ATA command block registers */
	ATA_HOB			= (1 << 7),	/* LBA48 selector */
	ATA_NIEN		= (1 << 1),	/* disable-irq flag */
	ATA_LBA			= (1 << 6),	/* LBA28 selector */
	ATA_DEV1		= (1 << 4),	/* Select Device 1 (slave) */
	ATA_DEVICE_OBS		= (1 << 7) | (1 << 5), /* obs bits in dev reg */
	ATA_DEVCTL_OBS		= (1 << 3),	/* obsolete bit in devctl reg */
	ATA_BUSY		= (1 << 7),	/* BSY status bit */
	ATA_DRDY		= (1 << 6),	/* device ready */
	ATA_DF			= (1 << 5),	/* device fault */
	ATA_DSC			= (1 << 4),	/* drive seek complete */
	ATA_DRQ			= (1 << 3),	/* data request i/o */
	ATA_CORR		= (1 << 2),	/* corrected data error */
	ATA_IDX			= (1 << 1),	/* index */
	ATA_ERR			= (1 << 0),	/* have an error */
	ATA_SRST		= (1 << 2),	/* software reset */
	ATA_ICRC		= (1 << 7),	/* interface CRC error */
	ATA_BBK			= ATA_ICRC,	/* pre-EIDE: block marked bad */
	ATA_UNC			= (1 << 6),	/* uncorrectable media error */
	ATA_MC			= (1 << 5),	/* media changed */
	ATA_IDNF		= (1 << 4),	/* ID not found */
	ATA_MCR			= (1 << 3),	/* media change requested */
	ATA_ABORTED		= (1 << 2),	/* command aborted */
	ATA_TRK0NF		= (1 << 1),	/* track 0 not found */
	ATA_AMNF		= (1 << 0),	/* address mark not found */
	ATAPI_LFS		= 0xF0,		/* last failed sense */
	ATAPI_EOM		= ATA_TRK0NF,	/* end of media */
	ATAPI_ILI		= ATA_AMNF,	/* illegal length indication */
	ATAPI_IO		= (1 << 1),
	ATAPI_COD		= (1 << 0),

	/* ATA command block registers */
	ATA_REG_DATA		= 0x00,
	ATA_REG_ERR		= 0x01,
	ATA_REG_NSECT		= 0x02,
	ATA_REG_LBAL		= 0x03,
	ATA_REG_LBAM		= 0x04,
	ATA_REG_LBAH		= 0x05,
	ATA_REG_DEVICE		= 0x06,
	ATA_REG_STATUS		= 0x07,

	ATA_REG_FEATURE		= ATA_REG_ERR, /* and their aliases */
	ATA_REG_CMD		= ATA_REG_STATUS,
	ATA_REG_BYTEL		= ATA_REG_LBAM,
	ATA_REG_BYTEH		= ATA_REG_LBAH,
	ATA_REG_DEVSEL		= ATA_REG_DEVICE,
	ATA_REG_IRQ		= ATA_REG_NSECT,

	/* ATA device commands */
	ATA_CMD_DEV_RESET	= 0x08, /* ATAPI device reset */
	ATA_CMD_CHK_POWER	= 0xE5, /* check power mode */
	ATA_CMD_STANDBY		= 0xE2, /* place in standby power mode */
	ATA_CMD_IDLE		= 0xE3, /* place in idle power mode */
	ATA_CMD_EDD		= 0x90,	/* execute device diagnostic */
	ATA_CMD_DOWNLOAD_MICRO  = 0x92,
	ATA_CMD_NOP		= 0x00,
	ATA_CMD_FLUSH		= 0xE7,
	ATA_CMD_FLUSH_EXT	= 0xEA,
	ATA_CMD_ID_ATA		= 0xEC,
	ATA_CMD_ID_ATAPI	= 0xA1,
	ATA_CMD_SERVICE		= 0xA2,
	ATA_CMD_READ		= 0xC8,
	ATA_CMD_READ_EXT	= 0x25,
	ATA_CMD_READ_QUEUED	= 0x26,
	ATA_CMD_READ_STREAM_EXT	= 0x2B,
	ATA_CMD_READ_STREAM_DMA_EXT = 0x2A,
	ATA_CMD_WRITE		= 0xCA,
	ATA_CMD_WRITE_EXT	= 0x35,
	ATA_CMD_WRITE_QUEUED	= 0x36,
	ATA_CMD_WRITE_STREAM_EXT = 0x3B,
	ATA_CMD_WRITE_STREAM_DMA_EXT = 0x3A,
	ATA_CMD_WRITE_FUA_EXT	= 0x3D,
	ATA_CMD_WRITE_QUEUED_FUA_EXT = 0x3E,
	ATA_CMD_FPDMA_READ	= 0x60,
	ATA_CMD_FPDMA_WRITE	= 0x61,
	ATA_CMD_PIO_READ	= 0x20,
	ATA_CMD_PIO_READ_EXT	= 0x24,
	ATA_CMD_PIO_WRITE	= 0x30,
	ATA_CMD_PIO_WRITE_EXT	= 0x34,
	ATA_CMD_READ_MULTI	= 0xC4,
	ATA_CMD_READ_MULTI_EXT	= 0x29,
	ATA_CMD_WRITE_MULTI	= 0xC5,
	ATA_CMD_WRITE_MULTI_EXT	= 0x39,
	ATA_CMD_WRITE_MULTI_FUA_EXT = 0xCE,
	ATA_CMD_SET_FEATURES	= 0xEF,
	ATA_CMD_SET_MULTI	= 0xC6,
	ATA_CMD_PACKET		= 0xA0,
	ATA_CMD_VERIFY		= 0x40,
	ATA_CMD_VERIFY_EXT	= 0x42,
	ATA_CMD_WRITE_UNCORR_EXT = 0x45,
	ATA_CMD_STANDBYNOW1	= 0xE0,
	ATA_CMD_IDLEIMMEDIATE	= 0xE1,
	ATA_CMD_SLEEP		= 0xE6,
	ATA_CMD_INIT_DEV_PARAMS	= 0x91,
	ATA_CMD_READ_NATIVE_MAX	= 0xF8,
	ATA_CMD_READ_NATIVE_MAX_EXT = 0x27,
	ATA_CMD_SET_MAX		= 0xF9,
	ATA_CMD_SET_MAX_EXT	= 0x37,
	ATA_CMD_READ_LOG_EXT	= 0x2F,
	ATA_CMD_WRITE_LOG_EXT	= 0x3F,
	ATA_CMD_READ_LOG_DMA_EXT = 0x47,
	ATA_CMD_WRITE_LOG_DMA_EXT = 0x57,
	ATA_CMD_TRUSTED_RCV	= 0x5C,
	ATA_CMD_TRUSTED_RCV_DMA = 0x5D,
	ATA_CMD_TRUSTED_SND	= 0x5E,
	ATA_CMD_TRUSTED_SND_DMA = 0x5F,
	ATA_CMD_PMP_READ	= 0xE4,
	ATA_CMD_PMP_WRITE	= 0xE8,
	ATA_CMD_CONF_OVERLAY	= 0xB1,
	ATA_CMD_SEC_SET_PASS	= 0xF1,
	ATA_CMD_SEC_UNLOCK	= 0xF2,
	ATA_CMD_SEC_ERASE_PREP	= 0xF3,
	ATA_CMD_SEC_ERASE_UNIT	= 0xF4,
	ATA_CMD_SEC_FREEZE_LOCK	= 0xF5,
	ATA_CMD_SEC_DISABLE_PASS = 0xF6,
	ATA_CMD_CONFIG_STREAM	= 0x51,
	ATA_CMD_SMART		= 0xB0,
	ATA_CMD_MEDIA_LOCK	= 0xDE,
	ATA_CMD_MEDIA_UNLOCK	= 0xDF,
	ATA_CMD_DSM		= 0x06,
	ATA_CMD_CHK_MED_CRD_TYP = 0xD1,
	ATA_CMD_CFA_REQ_EXT_ERR = 0x03,
	ATA_CMD_CFA_WRITE_NE	= 0x38,
	ATA_CMD_CFA_TRANS_SECT	= 0x87,
	ATA_CMD_CFA_ERASE	= 0xC0,
	ATA_CMD_CFA_WRITE_MULT_NE = 0xCD,
	/* marked obsolete in the ATA/ATAPI-7 spec */
	ATA_CMD_RESTORE		= 0x10,

	/* READ_LOG_EXT pages */
	ATA_LOG_SATA_NCQ	= 0x10,

	/* READ/WRITE LONG (obsolete) */
	ATA_CMD_READ_LONG	= 0x22,
	ATA_CMD_READ_LONG_ONCE	= 0x23,
	ATA_CMD_WRITE_LONG	= 0x32,
	ATA_CMD_WRITE_LONG_ONCE	= 0x33,

	/* SETFEATURES stuff */
	SETFEATURES_XFER	= 0x03,
	XFER_UDMA_7		= 0x47,
	XFER_UDMA_6		= 0x46,
	XFER_UDMA_5		= 0x45,
	XFER_UDMA_4		= 0x44,
	XFER_UDMA_3		= 0x43,
	XFER_UDMA_2		= 0x42,
	XFER_UDMA_1		= 0x41,
	XFER_UDMA_0		= 0x40,
	XFER_MW_DMA_4		= 0x24,	/* CFA only */
	XFER_MW_DMA_3		= 0x23,	/* CFA only */
	XFER_MW_DMA_2		= 0x22,
	XFER_MW_DMA_1		= 0x21,
	XFER_MW_DMA_0		= 0x20,
	XFER_SW_DMA_2		= 0x12,
	XFER_SW_DMA_1		= 0x11,
	XFER_SW_DMA_0		= 0x10,
	XFER_PIO_6		= 0x0E,	/* CFA only */
	XFER_PIO_5		= 0x0D,	/* CFA only */
	XFER_PIO_4		= 0x0C,
	XFER_PIO_3		= 0x0B,
	XFER_PIO_2		= 0x0A,
	XFER_PIO_1		= 0x09,
	XFER_PIO_0		= 0x08,
	XFER_PIO_SLOW		= 0x00,

	SETFEATURES_WC_ON	= 0x02, /* Enable write cache */
	SETFEATURES_WC_OFF	= 0x82, /* Disable write cache */

	/* Enable/Disable Automatic Acoustic Management */
	SETFEATURES_AAM_ON	= 0x42,
	SETFEATURES_AAM_OFF	= 0xC2,

	SETFEATURES_SPINUP	= 0x07, /* Spin-up drive */

	SETFEATURES_SATA_ENABLE = 0x10, /* Enable use of SATA feature */
	SETFEATURES_SATA_DISABLE = 0x90, /* Disable use of SATA feature */

	/* SETFEATURE Sector counts for SATA features */
	SATA_FPDMA_OFFSET	= 0x01,	/* FPDMA non-zero buffer offsets */
	SATA_FPDMA_AA		= 0x02, /* FPDMA Setup FIS Auto-Activate */
	SATA_DIPM		= 0x03,	/* Device Initiated Power Management */
	SATA_FPDMA_IN_ORDER	= 0x04,	/* FPDMA in-order data delivery */
	SATA_AN			= 0x05,	/* Asynchronous Notification */
	SATA_SSP		= 0x06,	/* Software Settings Preservation */

	/* feature values for SET_MAX */
	ATA_SET_MAX_ADDR	= 0x00,
	ATA_SET_MAX_PASSWD	= 0x01,
	ATA_SET_MAX_LOCK	= 0x02,
	ATA_SET_MAX_UNLOCK	= 0x03,
	ATA_SET_MAX_FREEZE_LOCK	= 0x04,

	/* feature values for DEVICE CONFIGURATION OVERLAY */
	ATA_DCO_RESTORE		= 0xC0,
	ATA_DCO_FREEZE_LOCK	= 0xC1,
	ATA_DCO_IDENTIFY	= 0xC2,
	ATA_DCO_SET		= 0xC3,

	/* feature values for SMART */
	ATA_SMART_ENABLE	= 0xD8,
	ATA_SMART_READ_VALUES	= 0xD0,
	ATA_SMART_READ_THRESHOLDS = 0xD1,

	/* feature values for Data Set Management */
	ATA_DSM_TRIM		= 0x01,

	/* password used in LBA Mid / LBA High for executing SMART commands */
	ATA_SMART_LBAM_PASS	= 0x4F,
	ATA_SMART_LBAH_PASS	= 0xC2,

	/* ATAPI stuff */
	ATAPI_PKT_DMA		= (1 << 0),
	ATAPI_DMADIR		= (1 << 2),	/* ATAPI data dir:
						   0=to device, 1=to host */
	ATAPI_CDB_LEN		= 16,

	/* PMP stuff */
	SATA_PMP_MAX_PORTS	= 15,
	SATA_PMP_CTRL_PORT	= 15,

	SATA_PMP_GSCR_DWORDS	= 128,
	SATA_PMP_GSCR_PROD_ID	= 0,
	SATA_PMP_GSCR_REV	= 1,
	SATA_PMP_GSCR_PORT_INFO	= 2,
	SATA_PMP_GSCR_ERROR	= 32,
	SATA_PMP_GSCR_ERROR_EN	= 33,
	SATA_PMP_GSCR_FEAT	= 64,
	SATA_PMP_GSCR_FEAT_EN	= 96,

	SATA_PMP_PSCR_STATUS	= 0,
	SATA_PMP_PSCR_ERROR	= 1,
	SATA_PMP_PSCR_CONTROL	= 2,

	SATA_PMP_FEAT_BIST	= (1 << 0),
	SATA_PMP_FEAT_PMREQ	= (1 << 1),
	SATA_PMP_FEAT_DYNSSC	= (1 << 2),
	SATA_PMP_FEAT_NOTIFY	= (1 << 3),

	/* cable types */
	ATA_CBL_NONE		= 0,
	ATA_CBL_PATA40		= 1,
	ATA_CBL_PATA80		= 2,
	ATA_CBL_PATA40_SHORT	= 3,	/* 40 wire cable to high UDMA spec */
	ATA_CBL_PATA_UNK	= 4,	/* don't know, maybe 80c? */
	ATA_CBL_PATA_IGN	= 5,	/* don't know, ignore cable handling */
	ATA_CBL_SATA		= 6,

	/* SATA Status and Control Registers */
	SCR_STATUS		= 0,
	SCR_ERROR		= 1,
	SCR_CONTROL		= 2,
	SCR_ACTIVE		= 3,
	SCR_NOTIFICATION	= 4,

	/* SError bits */
	SERR_DATA_RECOVERED	= (1 << 0), /* recovered data error */
	SERR_COMM_RECOVERED	= (1 << 1), /* recovered comm failure */
	SERR_DATA		= (1 << 8), /* unrecovered data error */
	SERR_PERSISTENT		= (1 << 9), /* persistent data/comm error */
	SERR_PROTOCOL		= (1 << 10), /* protocol violation */
	SERR_INTERNAL		= (1 << 11), /* host internal error */
	SERR_PHYRDY_CHG		= (1 << 16), /* PHY RDY changed */
	SERR_PHY_INT_ERR	= (1 << 17), /* PHY internal error */
	SERR_COMM_WAKE		= (1 << 18), /* Comm wake */
	SERR_10B_8B_ERR		= (1 << 19), /* 10b to 8b decode error */
	SERR_DISPARITY		= (1 << 20), /* Disparity */
	SERR_CRC		= (1 << 21), /* CRC error */
	SERR_HANDSHAKE		= (1 << 22), /* Handshake error */
	SERR_LINK_SEQ_ERR	= (1 << 23), /* Link sequence error */
	SERR_TRANS_ST_ERROR	= (1 << 24), /* Transport state trans. error */
	SERR_UNRECOG_FIS	= (1 << 25), /* Unrecognized FIS */
	SERR_DEV_XCHG		= (1 << 26), /* device exchanged */

	/* struct ata_taskfile flags */
	ATA_TFLAG_LBA48		= (1 << 0), /* enable 48-bit LBA and "HOB" */
	ATA_TFLAG_ISADDR	= (1 << 1), /* enable r/w to nsect/lba regs */
	ATA_TFLAG_DEVICE	= (1 << 2), /* enable r/w to device reg */
	ATA_TFLAG_WRITE		= (1 << 3), /* data dir: host->dev==1 (write) */
	ATA_TFLAG_LBA		= (1 << 4), /* enable LBA */
	ATA_TFLAG_FUA		= (1 << 5), /* enable FUA */
	ATA_TFLAG_POLLING	= (1 << 6), /* set nIEN to 1 and use polling */

	/* protocol flags */
	ATA_PROT_FLAG_PIO	= (1 << 0), /* is PIO */
	ATA_PROT_FLAG_DMA	= (1 << 1), /* is DMA */
	ATA_PROT_FLAG_DATA	= ATA_PROT_FLAG_PIO | ATA_PROT_FLAG_DMA,
	ATA_PROT_FLAG_NCQ	= (1 << 2), /* is NCQ */
	ATA_PROT_FLAG_ATAPI	= (1 << 3), /* is ATAPI */
};

#endif /* __ATA_H__ */
