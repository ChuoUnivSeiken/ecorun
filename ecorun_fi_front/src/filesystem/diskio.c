/*
 * diskio.c
 *
 *  Created on: 2015/10/08
 *      Author: Yoshio
 */

#include "../filesystem/diskio.h"

#include "../system/peripheral/ssp.h"
#include "../system/cmsis/LPC13Uxx.h"
#include "../system/common_types.h"
#include "../countdown_timer.h"

/* MMC/SD command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

static volatile DSTATUS stat = STA_NOINIT; /* Physical drive status */
static volatile BYTE card_type; /* Card type flags */

/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/
static void xmit_spi(BYTE dat)
{
	ssp_send(1, (uint8_t*) &dat, 1);
}

/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/
static BYTE rcvr_spi(void)
{
	BYTE data = 0;
	ssp_receive(1, &data, 1);
	return data;
}

static void rcvr_spi_m(uint8_t* dst)
{
	ssp_receive(1, (uint8_t*) dst, 1);
}

void sdcard_cs_high(void)
{
	LPC_GPIO->SET[1] |= _BV(23);
}

void sdcard_cs_low(void)
{
	LPC_GPIO->CLR[1] |= _BV(23);
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
static bool wait_ready(uint32_t wt)
{
	volatile uint8_t res;

	start_countdown(wt);
	do
	{
		ssp_receive(1, (uint8_t*) &res, 1);
		/* This loop takes a time. Insert rot_rdq() here for multitask envilonment. */
	} while ((res != 0xFF) && countdown_timer_val); /* Wait for card goes ready or timeout */

	return res == 0xFF;
}

/*-----------------------------------------------------------------------*/
/* Deselect card and release SPI                                         */
/*-----------------------------------------------------------------------*/
static void deselect(void)
{
	volatile uint8_t dummy;
	sdcard_cs_high(); /* Set CS# high */
	ssp_receive(1, (uint8_t*) &dummy, 1); /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select card and wait for ready                                        */
/*-----------------------------------------------------------------------*/
static int select(void) /* 1:OK, 0:Timeout */
{
	sdcard_cs_low(); /* Set CS# low */
	if (wait_ready(500))
	{
		return 1; /* Wait for card ready */
	}

	deselect();
	return 0; /* Timeout */
}

/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/

static void power_on(void)
{
}

static void power_off(void)
{
}

static int chk_power(void) /* Socket power state: 0=off, 1=on */
{
	return 1;
}

static uint8_t send_cmd(uint8_t type, uint32_t arg)
{
	volatile uint8_t buf, i;
	volatile uint32_t n_retry = 10;

	if (type & 0x80)
	{ /* Send a CMD55 prior to ACMD<n> */
		type &= 0x7F;
		buf = send_cmd(CMD55, 0);
		if (buf > 1)
		{
			return buf;
		}
	}

	if (type != CMD12)
	{
		deselect();
		if (!select())
		{
			return 0xFF;
		}
	}

	buf = 0x40 | type;
	ssp_send(1, (uint8_t*) &buf, 1);
	buf = (uint8_t) (arg >> 24); /* Argument[31..24] */
	ssp_send(1, (uint8_t*) &buf, 1);
	buf = (uint8_t) (arg >> 16); /* Argument[23..16] */
	ssp_send(1, (uint8_t*) &buf, 1);
	buf = (uint8_t) (arg >> 8); /* Argument[15..8] */
	ssp_send(1, (uint8_t*) &buf, 1);
	buf = (uint8_t) (arg); /* Argument[7..0] */
	ssp_send(1, (uint8_t*) &buf, 1);

	buf = 0x01;
	if (type == CMD0)
	{
		buf = 0x95; /* Valid CRC for CMD0(0) */
	}
	if (type == CMD8)
	{
		buf = 0x87; /* Valid CRC for CMD8(0x1AA) */
	}
	ssp_send(1, (uint8_t*) &buf, 1);

	do
	{
		ssp_receive(1, (uint8_t*) &buf, 1);
	} while ((buf & 0x80) && --n_retry);

	//sdcard_cs_high();

	return buf;
}

DSTATUS disk_initialize(BYTE pdrv)
{
	volatile uint32_t i;
	volatile uint8_t buf = 0xFF;
	volatile uint8_t cmd;
	volatile uint8_t res;
	volatile uint8_t ocr[4];

	if (pdrv)
	{
		return STA_NOINIT; /* Supports only drive 0 */
	}

	sdcard_cs_high();

	for (i = 0; i < 10; i++)
	{
		ssp_receive(1, (uint8_t*) &buf, 1);
	}

	card_type = 0;
	if (send_cmd(CMD0, 0) == 1)
	{
		if (send_cmd(CMD8, 0x1AA) == 1) /* SDv2? */
		{
			ssp_receive(1, (uint8_t*) ocr, 4);
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
			{
				volatile uint32_t n_retry = 100;
				while (--n_retry && send_cmd(ACMD41, 1UL << 30))
				{
					delay_ms(10); /* Wait for end of initialization with ACMD41(HCS) */
				}

				if (n_retry && (send_cmd(CMD58, 0) == 0))
				{ /* Check CCS bit in the OCR */

					ssp_receive(1, (uint8_t*) ocr, 4);
					card_type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; /* Card id SDv2 */
				}
			}
		}
		else /* Not SDv2 card */
		{
			if (send_cmd(ACMD41, 0) <= 1)
			{
				card_type = CT_SD1;
				cmd = ACMD41; /* SDv1 (ACMD41(0)) *//* SDv1 (ACMD41(0)) */
			}
			else
			{
				card_type = CT_MMC;
				cmd = CMD1; /* MMCv3 (CMD1(0)) */
			}

			volatile uint32_t n_retry = 100;

			while (--n_retry && send_cmd(cmd, 0))
			{
				delay_ms(10); /* Wait for end of initialization */
			}

			if (send_cmd(CMD16, 512) != 0) /* Set block length: 512 */
			{
				card_type = 0;
			}
		}
	}

	if (card_type)
	{
		ssp_clock_fast(1);
		stat &= ~STA_NOINIT; /* Clear STA_NOINIT flag */
	}
	else
	{
		stat = STA_NOINIT;
	}

	return stat;
}

DSTATUS disk_status(BYTE pdrv)
{
	if (pdrv)
	{
		return STA_NOINIT; /* Supports only single drive */
	}
	return stat;
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the MMC                                         */
/*-----------------------------------------------------------------------*/
static bool write_datablock(const BYTE *buf, BYTE token)
{
	BYTE resp, wc;

	if (!wait_ready(500))
	{
		return false;
	}

	xmit_spi(token); /* Xmit data token */
	if (token != 0xFD)
	{ /* Is data token */
		wc = 0;
		do
		{ /* Xmit the 512 byte data block to MMC */
			xmit_spi(*buf++);
			xmit_spi(*buf++);
		} while (--wc);
		xmit_spi(0xFF); /* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi(); /* Reveive data response */
		if ((resp & 0x1F) != 0x05) /* If not accepted, return with error */
		{
			return false;
		}
	}

	return true;
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the MMC                                    */
/*-----------------------------------------------------------------------*/
static bool read_datablock(BYTE *buf, UINT buf_size)
{
	BYTE token;

	start_countdown(200);
	do
	{ /* Wait for data packet in timeout of 200ms */
		ssp_receive(1, (uint8_t*) &token, 1);
	} while ((token == 0xFF) && countdown_timer_val);

	if (token != 0xFE)
	{
		return false; /* If not valid data token, retutn with error */
	}

	do
	{ /* Receive the data block into buffer */
		ssp_receive(1, (uint8_t*) buf, 4);
		buf += 4;
	} while (buf_size -= 4);

	ssp_receive(1, (uint8_t*) &token, 1); /* Discard CRC */
	ssp_receive(1, (uint8_t*) &token, 1);

	return true;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
	if (pdrv || !count)
	{
		return RES_PARERR;
	}
	if (stat & STA_NOINIT)
	{
		return RES_NOTRDY;
	}

	if (!(card_type & CT_BLOCK))
		sector *= 512; /* Convert to byte address if needed */

	if (count == 1)
	{ /* Single block read */
		if ((send_cmd(CMD17, sector) == 0) /* READ_SINGLE_BLOCK */
		&& read_datablock(buff, 512))
			count = 0;
	}
	else
	{ /* Multiple block read */
		if (send_cmd(CMD18, sector) == 0)
		{ /* READ_MULTIPLE_BLOCK */
			do
			{
				if (!read_datablock(buff, 512))
					break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
	if (pdrv || !count)
	{
		return RES_PARERR;
	}
	if (stat & STA_NOINIT)
	{
		return RES_NOTRDY;
	}
	if (stat & STA_PROTECT)
	{
		return RES_WRPRT;
	}

	if (!(card_type & CT_BLOCK))
	{
		sector *= 512; /* Convert to byte address if needed */
	}

	if (count == 1)
	{ /* Single block write */
		if ((send_cmd(CMD24, sector) == 0) /* WRITE_BLOCK */
		&& write_datablock(buff, 0xFE))
			count = 0;
	}
	else
	{ /* Multiple block write */
		if (card_type & CT_SDC)
			send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0)
		{ /* WRITE_MULTIPLE_BLOCK */
			do
			{
				if (!write_datablock(buff, 0xFC))
					break;
				buff += 512;
			} while (--count);
			if (!write_datablock(0, 0xFD)) /* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

DRESULT disk_ioctl(BYTE drv, /* Physical drive nmuber (0) */
BYTE cmd, /* Control code */
void *buff /* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	DWORD *dp, st, ed, csize;

	if (drv)
		return RES_PARERR; /* Check parameter */
	if (stat & STA_NOINIT)
		return RES_NOTRDY; /* Check if drive is ready */

	res = RES_ERROR;

	switch (cmd)
	{
	case CTRL_SYNC: /* Wait for end of internal write process of the drive */
		if (select())
			res = RES_OK;
		break;

	case GET_SECTOR_COUNT: /* Get drive capacity in unit of sector (DWORD) */
		if ((send_cmd(CMD9, 0) == 0) && read_datablock(csd, 16))
		{
			if ((csd[0] >> 6) == 1)
			{ /* SDC ver 2.00 */
				csize = csd[9] + ((WORD) csd[8] << 8)
						+ ((DWORD) (csd[7] & 63) << 16) + 1;
				*(DWORD*) buff = csize << 10;
			}
			else
			{ /* SDC ver 1.XX or MMC ver 3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1)
						+ 2;
				csize = (csd[8] >> 6) + ((WORD) csd[7] << 2)
						+ ((WORD) (csd[6] & 3) << 10) + 1;
				*(DWORD*) buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE: /* Get erase block size in unit of sector (DWORD) */
		if (card_type & CT_SD2)
		{ /* SDC ver 2.00 */
			if (send_cmd(ACMD13, 0) == 0)
			{ /* Read SD status */
				rcvr_spi();
				if (read_datablock(csd, 16))
				{ /* Read partial block */
					for (n = 64 - 16; n; n--)
						rcvr_spi(); /* Purge trailing data */
					*(DWORD*) buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		}
		else
		{ /* SDC ver 1.XX or MMC */
			if ((send_cmd(CMD9, 0) == 0) && read_datablock(csd, 16))
			{ /* Read CSD */
				if (card_type & CT_SD1)
				{ /* SDC ver 1.XX */
					*(DWORD*) buff = (((csd[10] & 63) << 1)
							+ ((WORD) (csd[11] & 128) >> 7) + 1)
							<< ((csd[13] >> 6) - 1);
				}
				else
				{ /* MMC */
					*(DWORD*) buff =
							((WORD) ((csd[10] & 124) >> 2) + 1)
									* (((csd[11] & 3) << 3)
											+ ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case CTRL_TRIM: /* Erase a block of sectors (used when _USE_TRIM == 1) */
		if (!(card_type & CT_SDC))
			break; /* Check if the card is SDC */
		if (disk_ioctl(drv, MMC_GET_CSD, csd))
			break; /* Get CSD */
		if (!(csd[0] >> 6) && !(csd[10] & 0x40))
			break; /* Check if sector erase can be applied to the card */
		dp = buff;
		st = dp[0];
		ed = dp[1]; /* Load sector block */
		if (!(card_type & CT_BLOCK))
		{
			st *= 512;
			ed *= 512;
		}
		if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0
				&& send_cmd(CMD38, 0) == 0 && wait_ready(30000)) /* Erase sector block */
			res = RES_OK; /* FatFs does not check result of this command */
		break;

		/* Following commands are never used by FatFs module */

	case MMC_GET_TYPE: /* Get MMC/SDC type (BYTE) */
		*ptr = card_type;
		res = RES_OK;
		break;

	case MMC_GET_CSD: /* Read CSD (16 bytes) */
		if (send_cmd(CMD9, 0) == 0 /* READ_CSD */
		&& read_datablock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_CID: /* Read CID (16 bytes) */
		if (send_cmd(CMD10, 0) == 0 /* READ_CID */
		&& read_datablock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_OCR: /* Read OCR (4 bytes) */
		if (send_cmd(CMD58, 0) == 0)
		{ /* READ_OCR */
			for (n = 4; n; n--)
				*ptr++ = rcvr_spi();
			res = RES_OK;
		}
		break;

	case MMC_GET_SDSTAT: /* Read SD status (64 bytes) */
		if (send_cmd(ACMD13, 0) == 0)
		{ /* SD_STATUS */
			rcvr_spi();
			if (read_datablock(ptr, 64))
				res = RES_OK;
		}
		break;

	default:
		res = RES_PARERR;
	}

	deselect();

	return res;
}

