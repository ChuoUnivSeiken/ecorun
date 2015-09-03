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
	LPC_SYSCON->SSP0CLKDIV = SCB_CLKDIV_DIV40;

	/* (PCLK / (CPSDVSR * [SCR+1])) = (4,800,000 / (2 x [5 + 1])) = 400 KHz */
	LPC_SSP0->CR0 = ((15u << 0)     // Data size = 16-bit  (bits 3:0)
	| (0 << 4)             // Frame format = SPI (bits 5:4)
#if SSP0_CPOL == 1
			| (1 << 6)            // CPOL = 1           (bit 6)
#else
			| (0 << 6)            // CPOL = 0           (bit 6)
#endif
#if SSP0_CPHA == 1
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
#if SSP0_CPOL == 1
			| (1 << 6)            // CPOL = 1           (bit 6)
#else
			| (0 << 6)            // CPOL = 0           (bit 6)
#endif
#if SSP0_CPHA == 1
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
	volatile uint8_t i, Dummy = Dummy;

	/* Reset SSP */
	LPC_SYSCON->PRESETCTRL &= ~0x1;
	LPC_SYSCON->PRESETCTRL |= 0x01;

	/* Enable AHB clock to the SSP domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11);

	/* Set SPI clock to slow-speed by default */
	ssp_clock_slow();

	/* Clear the Rx FIFO */
	for (i = 0; i < SSP0_FIFOSIZE; i++)
	{
		Dummy = LPC_SSP0->DR;
	}
	/* Enable the SSP Interrupt */
	//NVIC_EnableIRQ(SSP0_IRQn);
	/* Enable device and set it to master mode, no loopback */
	LPC_SSP0->CR1 = SSP0_CR1_SSE_ENABLED | SSP0_CR1_MS_MASTER
			| SSP0_CR1_LBM_NORMAL;
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
	volatile uint32_t i;
	volatile uint8_t Dummy = 0;

	for (i = 0; i < length; i++)
	{
		/* Move on only if NOT busy and TX FIFO not full. */
		while ((LPC_SSP0->SR & (SSP0_SR_TNF_NOTFULL | SSP0_SR_BSY_BUSY))
				!= SSP0_SR_TNF_NOTFULL)
			;

		LPC_SSP0->DR = *buf;
		buf++;

		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
				!= SSP0_SR_RNE_NOTEMPTY)
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
	volatile uint32_t i;

	for (i = 0; i < length; i++)
	{
		/* As long as the receive FIFO is not empty, data can be received. */
		LPC_SSP0->DR = 0xFFFF;

		/* Wait until the Busy bit is cleared */
		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
				!= SSP0_SR_RNE_NOTEMPTY)
			;

		*buf = LPC_SSP0->DR;
		buf++;
	}

	return;
}

void ssp_exchange(uint8_t *buf, uint32_t length)
{
	volatile uint32_t i;

	for (i = 0; i < length; i++)
	{
		/* As long as the receive FIFO is not empty, data can be received. */
		LPC_SSP0->DR = buf[i];

		/* Wait until the Busy bit is cleared */
		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
				!= SSP0_SR_RNE_NOTEMPTY)
			;

		buf[i] = LPC_SSP0->DR;
	}

	return;
}

void ssp_send2(uint16_t *buf, uint32_t length)
{
	volatile uint32_t i;
	volatile uint8_t Dummy = 0;

	for (i = 0; i < length; i++)
	{
		/* Move on only if NOT busy and TX FIFO not full. */
		while ((LPC_SSP0->SR & (SSP0_SR_TNF_NOTFULL | SSP0_SR_BSY_BUSY))
				!= SSP0_SR_TNF_NOTFULL)
			;

		LPC_SSP0->DR = *buf;
		buf++;

		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
				!= SSP0_SR_RNE_NOTEMPTY)
			;

		/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
		 on MISO. Otherwise, when sspReceive is called, previous data byte
		 is left in the FIFO. */
		Dummy = LPC_SSP0->DR;
	}

	return;
}

void ssp_receive2(uint16_t *buf, uint32_t length)
{
	volatile uint32_t i;

	for (i = 0; i < length; i++)
	{
		/* As long as the receive FIFO is not empty, data can be received. */
		LPC_SSP0->DR = 0xFF;

		/* Wait until the Busy bit is cleared */
		while ((LPC_SSP0->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
				!= SSP0_SR_RNE_NOTEMPTY)
			;

		*buf = LPC_SSP0->DR;
		buf++;
	}

	return;
}

