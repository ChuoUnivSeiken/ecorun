/*
 * i2c_handler.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "../type.h"
#include "i2c.h"

extern volatile uint32_t I2CMasterState;
extern volatile uint32_t I2CSlaveState;
extern volatile uint32_t timeout;

extern volatile uint32_t I2CMode;

extern volatile uint8_t I2CMasterBuffer[BUFSIZE];
extern volatile uint8_t I2CSlaveBuffer[BUFSIZE];
extern volatile uint32_t I2CCount;
extern volatile uint32_t I2CReadLength;
extern volatile uint32_t I2CWriteLength;

extern volatile uint32_t RdIndex;
extern volatile uint32_t WrIndex;

void I2C_IRQHandler(void) {
	uint8_t StatValue;

	timeout = 0;
	/* this handler deals with master read and master write only */
	StatValue = LPC_I2C->STAT;

	switch (StatValue) {
	case 0x08: /* A Start condition is issued. */
		WrIndex = 0;
		LPC_I2C->DAT = I2CMasterBuffer[WrIndex++];
		LPC_I2C->CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
		break;

	case 0x10: /* A repeated started is issued */
		RdIndex = 0;
		/* Send SLA with R bit set, */
		LPC_I2C->DAT = I2CMasterBuffer[WrIndex++];
		LPC_I2C->CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
		break;

	case 0x18: /* Regardless, it's a ACK */
		if (I2CWriteLength == 1) {
			LPC_I2C->CONSET = I2CONSET_STO; /* Set Stop flag */
			I2CMasterState = I2C_NO_DATA;
		} else {
			LPC_I2C->DAT = I2CMasterBuffer[WrIndex++];
		}
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;

	case 0x28: /* Data byte has been transmitted, regardless ACK or NACK */
		if (WrIndex < I2CWriteLength) {
			LPC_I2C->DAT = I2CMasterBuffer[WrIndex++]; /* this should be the last one */
		} else {
			if (I2CReadLength != 0) {
				LPC_I2C->CONSET = I2CONSET_STA; /* Set Repeated-start flag */
			} else {
				LPC_I2C->CONSET = I2CONSET_STO; /* Set Stop flag */
				I2CMasterState = I2C_OK;
			}
		}
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;

	case 0x30:
		LPC_I2C->CONSET = I2CONSET_STO; /* Set Stop flag */
		I2CMasterState = I2C_NACK_ON_DATA;
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;

	case 0x40: /* Master Receive, SLA_R has been sent */
		if ((RdIndex + 1) < I2CReadLength) {
			/* Will go to State 0x50 */
			LPC_I2C->CONSET = I2CONSET_AA; /* assert ACK after data is received */
		} else {
			/* Will go to State 0x58 */
			LPC_I2C->CONCLR = I2CONCLR_AAC; /* assert NACK after data is received */
		}
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;

	case 0x50: /* Data byte has been received, regardless following ACK or NACK */
		I2CSlaveBuffer[RdIndex++] = LPC_I2C->DAT;
		if ((RdIndex + 1) < I2CReadLength) {
			LPC_I2C->CONSET = I2CONSET_AA; /* assert ACK after data is received */
		} else {
			LPC_I2C->CONCLR = I2CONCLR_AAC; /* assert NACK on last byte */
		}
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;

	case 0x58:
		I2CSlaveBuffer[RdIndex++] = LPC_I2C->DAT;
		I2CMasterState = I2C_OK;
		LPC_I2C->CONSET = I2CONSET_STO; /* Set Stop flag */
		LPC_I2C->CONCLR = I2CONCLR_SIC; /* Clear SI flag */
		break;

	case 0x20: /* regardless, it's a NACK */
	case 0x48:
		LPC_I2C->CONSET = I2CONSET_STO; /* Set Stop flag */
		I2CMasterState = I2C_NACK_ON_ADDRESS;
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;

	case 0x38: /* Arbitration lost, in this example, we don't
	 deal with multiple master situation */
	default:
		I2CMasterState = I2C_ARBITRATION_LOST;
		LPC_I2C->CONCLR = I2CONCLR_SIC;
		break;
	}
	return;
}

