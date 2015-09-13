/*
 * i2c.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "../common_types.h"
#include "i2c.h"

volatile uint32_t I2CMasterState = I2C_IDLE;
volatile uint32_t I2CSlaveState = I2C_IDLE;
volatile uint32_t timeout = 0;

volatile uint32_t I2CMode;

volatile uint8_t I2CMasterBuffer[BUFSIZE];
volatile uint8_t I2CSlaveBuffer[BUFSIZE];
volatile uint32_t I2CCount = 0;
volatile uint32_t I2CReadLength;
volatile uint32_t I2CWriteLength;

volatile uint32_t RdIndex = 0;
volatile uint32_t WrIndex = 0;

uint32_t i2c_start(void) {
	uint32_t timeout = 0;
	uint32_t retVal = false;

	/*--- Issue a start condition ---*/
	LPC_I2C->CONSET = I2CONSET_STA; /* Set Start flag */

	/*--- Wait until START transmitted ---*/
	while (1) {
		if (I2CMasterState == I2C_STARTED) {
			retVal = true;
			break;
		}
		if (timeout >= MAX_TIMEOUT) {
			retVal = false;
			break;
		}
		timeout++;
	}
	return (retVal);
}

uint32_t i2c_stop(void) {
	LPC_I2C->CONSET = I2CONSET_STO; /* Set Stop flag */
	LPC_I2C->CONCLR = I2CONCLR_SIC; /* Clear SI flag */

	/*--- Wait for STOP detected ---*/
	while (LPC_I2C->CONSET & I2CONSET_STO)
		;
	return true;
}

uint32_t i2c_init(uint32_t I2cMode) {
	LPC_SYSCON->PRESETCTRL |= (0x1 << 1);

	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 5);
	LPC_IOCON->PIO0_4 &= ~0x3F; /*  I2C I/O config */
	LPC_IOCON->PIO0_4 |= 0x01; /* I2C SCL */
	LPC_IOCON->PIO0_5 &= ~0x3F;
	LPC_IOCON->PIO0_5 |= 0x01; /* I2C SDA */
	/* IOCON may change in the next release, save change for future references. */
//  LPC_IOCON->PIO0_4 |= (0x1<<10);	/* open drain pins */
//  LPC_IOCON->PIO0_5 |= (0x1<<10);	/* open drain pins */
	/*--- Clear flags ---*/
	LPC_I2C->CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC
			| I2CONCLR_I2ENC;

	/*--- Reset registers ---*/
#if FAST_MODE_PLUS
	LPC_IOCON->PIO0_4 |= (0x2<<8);
	LPC_IOCON->PIO0_5 |= (0x2<<8);
	LPC_I2C->SCLL = I2SCLL_HS_SCLL;
	LPC_I2C->SCLH = I2SCLH_HS_SCLH;
#else
	LPC_I2C->SCLL = I2SCLL_SCLL;
	LPC_I2C->SCLH = I2SCLH_SCLH;
#endif

	if (I2cMode == I2CSLAVE) {
		LPC_I2C->ADR0 = PCF8594_ADDR;
	}

	/* Enable the I2C Interrupt */
	NVIC_EnableIRQ(I2C_IRQn);

	LPC_I2C->CONSET = I2CONSET_I2EN;
	return true;
}

uint32_t i2c_engine(void) {
	RdIndex = 0;
	WrIndex = 0;

	/*--- Issue a start condition ---*/
	LPC_I2C->CONSET = I2CONSET_STA; /* Set Start flag */

	I2CMasterState = I2C_BUSY;
	while (I2CMasterState == I2C_BUSY) {
		if (timeout >= MAX_TIMEOUT) {
			I2CMasterState = I2C_TIME_OUT;
			break;
		}
		timeout++;
	}
	LPC_I2C->CONCLR = I2CONCLR_STAC;

	return (I2CMasterState);
}

