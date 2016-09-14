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

/* SSP Clock div (0..255) */
#define SCB_CLKDIV_DISABLE (0x00000000)
#define SCB_CLKDIV_DIV1    (0x00000001)
#define SCB_CLKDIV_DIV2    (0x00000002)
#define SCB_CLKDIV_DIV3    (0x00000003)
#define SCB_CLKDIV_DIV4    (0x00000004)
#define SCB_CLKDIV_DIV6    (0x00000006)
#define SCB_CLKDIV_DIV10   (0x0000000A)
#define SCB_CLKDIV_DIV12   (0x0000000C)
#define SCB_CLKDIV_DIV15   (0x0000000F)
#define SCB_CLKDIV_DIV20   (0x00000014)
#define SCB_CLKDIV_DIV40   (0x00000028)
#define SCB_CLKDIV_MASK    (0x000000FF)

/* SSP Serial Clock Rate Values */
#define SSP0_SCR_1          (0x00000100)
#define SSP0_SCR_2          (0x00000200)
#define SSP0_SCR_3          (0x00000300)
#define SSP0_SCR_4          (0x00000400)
#define SSP0_SCR_5          (0x00000500)
#define SSP0_SCR_6          (0x00000600)
#define SSP0_SCR_7          (0x00000700)
#define SSP0_SCR_8          (0x00000800)
#define SSP0_SCR_9          (0x00000900)
#define SSP0_SCR_10         (0x00000A00)
#define SSP0_SCR_11         (0x00000B00)
#define SSP0_SCR_12         (0x00000C00)
#define SSP0_SCR_13         (0x00000D00)
#define SSP0_SCR_14         (0x00000E00)
#define SSP0_SCR_15         (0x00000F00)
#define SSP0_SCR_16         (0x00001000)

/*  Current status of the SSP controller.  */
#define SSP0_SR_TFE_MASK      (0x00000001) // Transmit FIFO empty
#define SSP0_SR_TFE_EMPTY     (0x00000001)
#define SSP0_SR_TFE_NOTEMPTY  (0x00000000)
#define SSP0_SR_TNF_MASK      (0x00000002) // Transmit FIFO not full
#define SSP0_SR_TNF_NOTFULL   (0x00000002)
#define SSP0_SR_TNF_FULL      (0x00000000)
#define SSP0_SR_RNE_MASK      (0x00000004) // Receive FIFO not empty
#define SSP0_SR_RNE_NOTEMPTY  (0x00000004)
#define SSP0_SR_RNE_EMPTY     (0x00000000)
#define SSP0_SR_RFF_MASK      (0x00000008) // Receive FIFO full
#define SSP0_SR_RFF_FULL      (0x00000008)
#define SSP0_SR_RFF_NOTFULL   (0x00000000)
#define SSP0_SR_BSY_MASK      (0x00000010) // Busy Flag
#define SSP0_SR_BSY_IDLE      (0x00000000)
#define SSP0_SR_BSY_BUSY      (0x00000010)

/* Control Register 1 */
#define SSP0_CR1_LBM_MASK     (0x00000001) // Loop back mode
#define SSP0_CR1_LBM_NORMAL   (0x00000000)
#define SSP0_CR1_LBM_INVERTED (0x00000001) // MISO/MOSI are reversed
#define SSP0_CR1_SSE_MASK     (0x00000002) // SSP enable
#define SSP0_CR1_SSE_DISABLED (0x00000000)
#define SSP0_CR1_SSE_ENABLED  (0x00000002)
#define SSP0_CR1_MS_MASK      (0x00000004) // Master/Slave Mode
#define SSP0_CR1_MS_MASTER    (0x00000000)
#define SSP0_CR1_MS_SLAVE     (0x00000004)
#define SSP0_CR1_SOD_MASK     (0x00000008) // Slave output disable

/* SSP Serial Clock Rate Values */
#define SSP1_SCR_1          (0x00000100)
#define SSP1_SCR_2          (0x00000200)
#define SSP1_SCR_3          (0x00000300)
#define SSP1_SCR_4          (0x00000400)
#define SSP1_SCR_5          (0x00000500)
#define SSP1_SCR_6          (0x00000600)
#define SSP1_SCR_7          (0x00000700)
#define SSP1_SCR_8          (0x00000800)
#define SSP1_SCR_9          (0x00000900)
#define SSP1_SCR_10         (0x00000A00)
#define SSP1_SCR_11         (0x00000B00)
#define SSP1_SCR_12         (0x00000C00)
#define SSP1_SCR_13         (0x00000D00)
#define SSP1_SCR_14         (0x00000E00)
#define SSP1_SCR_15         (0x00000F00)
#define SSP1_SCR_16         (0x00001000)

/*  Current status of the SSP controller.  */
#define SSP1_SR_TFE_MASK      (0x00000001) // Transmit FIFO empty
#define SSP1_SR_TFE_EMPTY     (0x00000001)
#define SSP1_SR_TFE_NOTEMPTY  (0x00000000)
#define SSP1_SR_TNF_MASK      (0x00000002) // Transmit FIFO not full
#define SSP1_SR_TNF_NOTFULL   (0x00000002)
#define SSP1_SR_TNF_FULL      (0x00000000)
#define SSP1_SR_RNE_MASK      (0x00000004) // Receive FIFO not empty
#define SSP1_SR_RNE_NOTEMPTY  (0x00000004)
#define SSP1_SR_RNE_EMPTY     (0x00000000)
#define SSP1_SR_RFF_MASK      (0x00000008) // Receive FIFO full
#define SSP1_SR_RFF_FULL      (0x00000008)
#define SSP1_SR_RFF_NOTFULL   (0x00000000)
#define SSP1_SR_BSY_MASK      (0x00000010) // Busy Flag
#define SSP1_SR_BSY_IDLE      (0x00000000)
#define SSP1_SR_BSY_BUSY      (0x00000010)

/* Control Register 1 */
#define SSP1_CR1_LBM_MASK     (0x00000001) // Loop back mode
#define SSP1_CR1_LBM_NORMAL   (0x00000000)
#define SSP1_CR1_LBM_INVERTED (0x00000001) // MISO/MOSI are reversed
#define SSP1_CR1_SSE_MASK     (0x00000002) // SSP enable
#define SSP1_CR1_SSE_DISABLED (0x00000000)
#define SSP1_CR1_SSE_ENABLED  (0x00000002)
#define SSP1_CR1_MS_MASK      (0x00000004) // Master/Slave Mode
#define SSP1_CR1_MS_MASTER    (0x00000000)
#define SSP1_CR1_MS_SLAVE     (0x00000004)
#define SSP1_CR1_SOD_MASK     (0x00000008) // Slave output disable

/**************************************************************************/
/*!
 Set SSP clock to slow (400 KHz)
 */
/**************************************************************************/
void ssp_clock_slow(uint32_t port)
{
	if (port == 0)
	{
		/* Divide by 15 for SSPCLKDIV */
		LPC_SYSCON->SSP0CLKDIV = SCB_CLKDIV_DIV40;

		/* (PCLK / (CPSDVSR * [SCR+1])) = (4,800,000 / (2 x [5 + 1])) = 400 KHz */
		LPC_SSP0->CR0 = (((SSP0_DATASIZE - 1) << 0) // Data size = 16-bit  (bits 3:0)
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

		return;
	}
	else
	{
		/* Divide by 15 for SSPCLKDIV */
		LPC_SYSCON->SSP1CLKDIV = SCB_CLKDIV_DIV15;

		/* (PCLK / (CPSDVSR * [SCR+1])) = (4,800,000 / (2 x [5 + 1])) = 400 KHz */
		LPC_SSP1->CR0 = (((SSP1_DATASIZE - 1) << 0) // Data size = 8-bit  (bits 3:0)
		| (0 << 4)             // Frame format = SPI (bits 5:4)
#if SSP1_CPOL == 1
				| (1 << 6)            // CPOL = 1           (bit 6)
#else
				| (0 << 6)            // CPOL = 0           (bit 6)
#endif
#if SSP1_CPHA == 1
				| (1 << 7)             // CPHA = 1           (bit 7)
#else
				| (0 << 7)             // CPHA = 0           (bit 7)
#endif
				| SSP1_SCR_5);         // Clock rate = 5     (bits 15:8)

		/* Clock prescale register must be even and at least 2 in master mode */
		LPC_SSP1->CPSR = 2;

		return;
	}
}

/**************************************************************************/
/*!
 Set SSP clock to fast (6.0 MHz)
 */
/**************************************************************************/
void ssp_clock_fast(uint32_t port)
{
	if (port == 0)
	{
		/* Divide by 1 for SSPCLKDIV */
		LPC_SYSCON->SSP0CLKDIV = SCB_CLKDIV_DIV1;

		/* (PCLK / (CPSDVSR * [SCR+1])) = (72,000,000 / (2 * [5 + 1])) = 6.0 MHz */
		LPC_SSP0->CR0 = (((SSP0_DATASIZE - 1) << 0) // Data size = 16-bit  (bits 3:0)
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
	else
	{
		/* Divide by 1 for SSPCLKDIV */
		LPC_SYSCON->SSP1CLKDIV = SCB_CLKDIV_DIV1;

		/* (PCLK / (CPSDVSR * [SCR+1])) = (72,000,000 / (2 * [5 + 1])) = 6.0 MHz */
		LPC_SSP1->CR0 = (((SSP1_DATASIZE - 1) << 0) // Data size = 8-bit  (bits 3:0)
		| (0 << 4)             // Frame format = SPI (bits 5:4)
#if SSP1_CPOL == 1
				| (1 << 6)            // CPOL = 1           (bit 6)
#else
				| (0 << 6)            // CPOL = 0           (bit 6)
#endif
#if SSP1_CPHA == 1
				| (1 << 7)             // CPHA = 1           (bit 7)
#else
				| (0 << 7)             // CPHA = 0           (bit 7)
#endif
				| SSP0_SCR_5);         // Clock rate = 5     (bits 15:8)

		/* Clock prescale register must be even and at least 2 in master mode */
		LPC_SSP1->CPSR = 2;
	}
}

/**************************************************************************/
/*!
 @brief Initialise SSP0
 */
/**************************************************************************/
void ssp_init(uint32_t port)
{
	volatile uint8_t i, dummy = 0;

	if (port == 0)
	{
		/* Reset SSP */
		LPC_SYSCON->PRESETCTRL &= ~0x1;
		LPC_SYSCON->PRESETCTRL |= 0x01;

		/* Enable AHB clock to the SSP domain. */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11);

		/* Set SPI clock to slow-speed by default */
		ssp_clock_slow(port);

		/* Clear the Rx FIFO */
		for (i = 0; i < SSP0_FIFOSIZE; i++)
		{
			dummy = LPC_SSP0->DR;
		}
		/* Enable the SSP Interrupt */
		//NVIC_EnableIRQ(SSP0_IRQn);
		/* Enable device and set it to master mode, no loopback */
		LPC_SSP0->CR1 = SSP0_CR1_SSE_ENABLED | SSP0_CR1_MS_MASTER
				| SSP0_CR1_LBM_NORMAL;

		return;
	}
	else
	{
		/* Reset SSP */
		LPC_SYSCON->PRESETCTRL |= (0x1 << 2);

		/* Enable AHB clock to the SSP domain. */
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 18);

		/* Set SPI clock to slow-speed by default */
		ssp_clock_slow(port);

		/* Clear the Rx FIFO */
		for (i = 0; i < SSP1_FIFOSIZE; i++)
		{
			dummy = LPC_SSP1->DR;
		}

		/* Enable device and set it to master mode, no loopback */
		LPC_SSP1->CR1 = SSP1_CR1_SSE_ENABLED | SSP1_CR1_MS_MASTER
				| SSP1_CR1_LBM_NORMAL;

		return;
	}
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
void ssp_send(uint32_t port, const uint8_t *buf, uint32_t length)
{
	volatile uint32_t i;
	volatile uint8_t dummy = 0;

	if (port == 0)
	{
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
			dummy = LPC_SSP0->DR;
		}

		return;
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			/* Move on only if NOT busy and TX FIFO not full. */
			while ((LPC_SSP1->SR & (SSP1_SR_TNF_NOTFULL | SSP1_SR_BSY_BUSY))
					!= SSP1_SR_TNF_NOTFULL)
				;
			LPC_SSP1->DR = *buf;
			buf++;

			while ((LPC_SSP1->SR & (SSP1_SR_BSY_BUSY | SSP1_SR_RNE_NOTEMPTY))
					!= SSP1_SR_RNE_NOTEMPTY)
				;
			/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
			 on MISO. Otherwise, when sspReceive is called, previous data byte
			 is left in the FIFO. */
			dummy = LPC_SSP1->DR;
		}

		return;
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
void ssp_receive(uint32_t port, uint8_t *buf, uint32_t length)
{
	volatile uint32_t i;

	if (port == 0)
	{
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
	else
	{
		for (i = 0; i < length; i++)
		{
			/* As long as the receive FIFO is not empty, data can be received. */
			LPC_SSP1->DR = 0xFF;

			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP1->SR & (SSP1_SR_BSY_BUSY | SSP1_SR_RNE_NOTEMPTY))
					!= SSP1_SR_RNE_NOTEMPTY)
				;

			*buf = LPC_SSP1->DR;
			buf++;
		}

		return;
	}

	return;
}

void ssp_exchange(uint32_t port, uint8_t *buf, uint32_t length)
{
	volatile uint32_t i;

	if (port == 0)
	{
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
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			/* As long as the receive FIFO is not empty, data can be received. */
			LPC_SSP1->DR = buf[i];

			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP1->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
					!= SSP0_SR_RNE_NOTEMPTY)
				;

			buf[i] = LPC_SSP1->DR;
		}
	}

	return;
}

void ssp_send_uint16(uint32_t port, const uint16_t *buf, uint32_t length)
{
	volatile uint32_t i;
	volatile uint8_t dummy = 0;

	if (port == 0)
	{
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
			dummy = LPC_SSP0->DR;
		}
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			/* Move on only if NOT busy and TX FIFO not full. */
			while ((LPC_SSP1->SR & (SSP0_SR_TNF_NOTFULL | SSP0_SR_BSY_BUSY))
					!= SSP0_SR_TNF_NOTFULL)
				;

			LPC_SSP1->DR = *buf;
			buf++;

			while ((LPC_SSP1->SR & (SSP0_SR_BSY_BUSY | SSP0_SR_RNE_NOTEMPTY))
					!= SSP0_SR_RNE_NOTEMPTY)
				;

			/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
			 on MISO. Otherwise, when sspReceive is called, previous data byte
			 is left in the FIFO. */
			dummy = LPC_SSP1->DR;
		}
	}

	return;
}

void ssp_receive_uint16(uint32_t port, uint16_t *buf, uint32_t length)
{
	volatile uint32_t i;

	if (port == 0)
	{
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
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			/* As long as the receive FIFO is not empty, data can be received. */
			LPC_SSP1->DR = 0xFFFF;

			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP1->SR & (SSP1_SR_BSY_BUSY | SSP1_SR_RNE_NOTEMPTY))
					!= SSP1_SR_RNE_NOTEMPTY)
				;

			*buf = LPC_SSP1->DR;
			buf++;
		}
	}

	return;
}

