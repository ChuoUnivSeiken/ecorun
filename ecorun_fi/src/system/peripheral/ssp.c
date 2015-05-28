/**************************************************************************/
/*!
 @file     ssp0.c
 @author   K. Townsend (microBuilder.eu)

 @section LICENSE

 Software License Agreement (BSD License)

 Copyright (c) 2012, K. Townsend (microBuilder.eu)
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holders nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**************************************************************************/
#include "ssp.h"
#include "../cmsis/LPC13Uxx.h"

/**************************************************************************/
/*!
 Set SSP clock to slow (400 KHz)
 */
/**************************************************************************/
void ssp_clock_slow()
{
	/* Divide by 15 for SSPCLKDIV */
	LPC_SYSCON->SSP0CLKDIV = SCB_CLKDIV_DIV15;

	/* (PCLK / (CPSDVSR * [SCR+1])) = (4,800,000 / (2 x [5 + 1])) = 400 KHz */
	LPC_SSP0->CR0 = ((7u << 0)     // Data size = 8-bit  (bits 3:0)
	| (0 << 4)             // Frame format = SPI (bits 5:4)
#if CFG_SSP_CPOL0 == 1
			| (1 << 6)            // CPOL = 1           (bit 6)
#else
			| (0 << 6)            // CPOL = 0           (bit 6)
#endif
#if CFG_SSP_CPHA0 == 1
			| (1 << 7)             // CPHA = 1           (bit 7)
#else
			| (0 << 7)             // CPHA = 0           (bit 7)
#endif
			| SSP0_SCR_5);         // Clock rate = 5     (bits 15:8)

	/* Clock prescale register must be even and at least 2 in master mode */
	LPC_SSP0->CPSR = 2;
}

/**************************************************************************/
/*!
 Set SSP clock to fast (6.0 MHz)
 */
/**************************************************************************/
void ssp_clock_fast()
{
	/* Divide by 1 for SSPCLKDIV */
	LPC_SYSCON->SSP0CLKDIV = SCB_CLKDIV_DIV1;

	/* (PCLK / (CPSDVSR * [SCR+1])) = (72,000,000 / (2 * [5 + 1])) = 6.0 MHz */
	LPC_SSP0->CR0 = ((7u << 0)     // Data size = 8-bit  (bits 3:0)
	| (0 << 4)             // Frame format = SPI (bits 5:4)
#if CFG_SSP_CPOL0 == 1
			| (1 << 6)            // CPOL = 1           (bit 6)
#else
			| (0 << 6)            // CPOL = 0           (bit 6)
#endif
#if CFG_SSP_CPHA0 == 1
			| (1 << 7)             // CPHA = 1           (bit 7)
#else
			| (0 << 7)             // CPHA = 0           (bit 7)
#endif
			| SSP0_SCR_5);         // Clock rate = 5     (bits 15:8)

	/* Clock prescale register must be even and at least 2 in master mode */
	LPC_SSP0->CPSR = 2;
}

/**************************************************************************/
/*!
 @brief Initialise SSP0
 */
/**************************************************************************/
void ssp_init(void)
{
	uint8_t i, Dummy = Dummy;

	/* Reset SSP */
	LPC_SYSCON->PRESETCTRL &= ~0x1;
	LPC_SYSCON->PRESETCTRL |= 0x01;

	/* Enable AHB clock to the SSP domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11);

	/* Set P0.8 to SSP MISO0 */
	LPC_IOCON->PIO0_8 &= ~0x07;
	LPC_IOCON->PIO0_8 |= 0x01;

	/* Set P0.9 to SSP MOSI0 */
	LPC_IOCON->PIO0_9 &= ~0x07;
	LPC_IOCON->PIO0_9 |= 0x01;

	/* No LPC_IOCON->SCKLOC register on LPC11Uxx/13Uxx? */
#if (SSP0_SCK0_LOCATION == SSP0_SCK0_1_29)
	/* Set 1.29 to SSP SCK0 (0.6 is often used by USB and 0.10 for SWD) */
	LPC_IOCON->PIO1_29 = 0x01;
#elif (SSP0_SCK0_LOCATION == SSP0_SCK0_0_10)
	/* Set 0.10 to SSP SCK0 (may be required for SWD!) */
	LPC_IOCON->SWCLK_PIO0_10 = 0x02;
#elif (SSP0_SCK0_LOCATION == SSP0_SCK0_0_6)
	/* Set 0.6 to SSP SCK0 (may be required for USB!) */
	LPC_IOCON->PIO0_6 = 0x02;
#else
#error "Invalid CFG_SSP_SCK0_LOCATION"
#endif

	/* Set SPI clock to high-speed by default */
	ssp_clock_fast();

	/* Clear the Rx FIFO */
	for (i = 0; i < SSP0_FIFOSIZE; i++)
	{
		Dummy = LPC_SSP0->DR;
	}
	/* Enable the SSP Interrupt */
	NVIC_EnableIRQ(SSP0_IRQn);

	/* Enable device and set it to master mode, no loopback */
	LPC_SSP0->CR1 = SSP0_CR1_SSE_ENABLED | SSP0_CR1_MS_MASTER | SSP0_CR1_LBM_NORMAL;
}

/**************************************************************************/
/*!
 @brief Sends a block of data using SSP0

 @param[in]  buf
 Pointer to the data buffer
 @param[in]  length
 Block length of the data buffer
 */
/**************************************************************************/
void ssp_send(uint8_t *buf, uint32_t length)
{
	uint32_t i;
	uint8_t Dummy = Dummy;

	for (i = 0; i < length; i++)
	{
		/* Move on only if NOT busy and TX FIFO not full. */
		while ((LPC_SSP0->SR & (SSP0_SR_TNF_NOTFULL | SSP0_SR_BSY_BUSY)) != SSP0_SR_TNF_NOTFULL)
			;
		LPC_SSP0->DR = *buf;
		buf++;

		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY)) != SSP0_SR_RNE_NOTEMPTY)
			;
		/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
		 on MISO. Otherwise, when sspReceive is called, previous data byte
		 is left in the FIFO. */
		Dummy = LPC_SSP0->DR;
	}

	return;
}

/**************************************************************************/
/*!
 @brief Receives a block of data using SSP0

 @param[in]  buf
 Pointer to the data buffer
 @param[in]  length
 Block length of the data buffer
 */
/**************************************************************************/
void ssp_receive(uint8_t *buf, uint32_t length)
{
	uint32_t i;

	for (i = 0; i < length; i++)
	{
		/* As long as the receive FIFO is not empty, data can be received. */
		LPC_SSP0->DR = 0xFF;

		/* Wait until the Busy bit is cleared */
		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY)) != SSP0_SR_RNE_NOTEMPTY)
			;

		*buf = LPC_SSP0->DR;
		buf++;
	}

	return;
}

/* SSP Interrupt Mask Set/Clear register */
#define SSPIMSC_RORIM   (0x1<<0)
#define SSPIMSC_RTIM    (0x1<<1)
#define SSPIMSC_RXIM    (0x1<<2)
#define SSPIMSC_TXIM    (0x1<<3)

/* SSP0 Interrupt Status register */
#define SSPRIS_RORRIS   (0x1<<0)
#define SSPRIS_RTRIS    (0x1<<1)
#define SSPRIS_RXRIS    (0x1<<2)
#define SSPRIS_TXRIS    (0x1<<3)

/* SSP0 Masked Interrupt register */
#define SSPMIS_RORMIS   (0x1<<0)
#define SSPMIS_RTMIS    (0x1<<1)
#define SSPMIS_RXMIS    (0x1<<2)
#define SSPMIS_TXMIS    (0x1<<3)

/* SSP0 Interrupt clear register */
#define SSPICR_RORIC    (0x1<<0)
#define SSPICR_RTIC     (0x1<<1)

/* statistics of all the interrupts */
static volatile uint32_t interruptRxStat0 = 0;
static volatile uint32_t interruptOverRunStat0 = 0;
static volatile uint32_t interruptRxTimeoutStat0 = 0;

static volatile uint32_t interruptRxStat1 = 0;
static volatile uint32_t interruptOverRunStat1 = 0;
static volatile uint32_t interruptRxTimeoutStat1 = 0;

#if 0
void SSP0_IRQHandler(void)
{
	uint32_t regValue;

	regValue = LPC_SSP0->MIS;
	if (regValue & SSPMIS_RORMIS) /* Receive overrun interrupt */
	{
		interruptOverRunStat0++;
		LPC_SSP0->ICR = SSPICR_RORIC; /* clear interrupt */
	}
	if (regValue & SSPMIS_RTMIS) /* Receive timeout interrupt */
	{
		interruptRxTimeoutStat0++;
		LPC_SSP0->ICR = SSPICR_RTIC; /* clear interrupt */
	}

	/* please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
	 are shared as global variables. It may create some race condition that main
	 and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
	 in both main and ISR could prevent this kind of race condition */
	if (regValue & SSPMIS_RXMIS) /* Rx at least half full */
	{
		interruptRxStat0++; /* receive until it's empty */
	}

}
#endif
