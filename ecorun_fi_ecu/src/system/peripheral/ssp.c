/*
 * ssp.c
 *
 *  Created on: 2015/08/16
 *      Author: Yoshio
 */

#include "ssp.h"
#include "../cmsis/LPC11xx.h"

#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t xSSP1RxQueue = NULL;
QueueHandle_t xSSP1TxQueue = NULL;

void ssp_init(uint8_t port)
{
	volatile uint8_t i;
	volatile uint32_t dummy;
	if (port == 1)
	{
		LPC_SYSCON->PRESETCTRL &= ~(0x1 << 2);
		LPC_SYSCON->PRESETCTRL |= (0x1 << 2);
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 18);
		LPC_SYSCON->SSP1CLKDIV = 0x02; /* Divided by 2 */

		LPC_IOCON->PIO2_2 &= ~0x07; /*  SSP I/O config */
		LPC_IOCON->PIO2_2 |= 0x02; /* SSP MISO */
		LPC_IOCON->PIO2_3 &= ~0x07;
		LPC_IOCON->PIO2_3 |= 0x02; /* SSP MOSI */
		LPC_IOCON->PIO2_1 &= ~0x07;
		LPC_IOCON->PIO2_1 |= 0x02; /* SSP CLK */

		LPC_IOCON->PIO2_0 &= ~0x07;
		LPC_IOCON->PIO2_0 |= 0x02; /* SSP SSEL */

		LPC_SSP1->CR0 = ((15u << 0)     // Data size = 16-bit  (bits 3:0)
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
				| 0x00000500);         // Clock rate = 5     (bits 15:8)

		/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
		LPC_SSP1->CPSR = 0x2;

		for (i = 0; i < FIFOSIZE; i++)
		{
			dummy = LPC_SSP1->DR; /* clear the RxFIFO */
		}

		/* Slave mode */
		if (LPC_SSP1->CR1 & SSPCR1_SSE)
		{
			/* The slave bit can't be set until SSE bit is zero. */
			LPC_SSP1->CR1 &= ~SSPCR1_SSE;
		}
		LPC_SSP1->CR1 = SSPCR1_SSE | SSPCR1_MS; /* Enable slave bit first */

		LPC_SSP1->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM | SSPIMSC_RXIM;

		xSSP1RxQueue = xQueueCreate(512 + 256, sizeof(uint16_t));
		xSSP1TxQueue = xQueueCreate(512, sizeof(uint16_t));

		/* Enable the SSP Interrupt */
		NVIC_EnableIRQ(SSP1_IRQn);
	}
	else
	{
		LPC_SYSCON->PRESETCTRL |= (0x1 << 0);
		LPC_SYSCON->SYSAHBCLKCTRL |= (0x1 << 11);
		LPC_SYSCON->SSP0CLKDIV = 0x01; /* Divided by 2 */

		LPC_IOCON->PIO0_8 &= ~0x07; /*  SSP I/O config */
		LPC_IOCON->PIO0_8 |= 0x01; /* SSP MISO */
		LPC_IOCON->PIO0_9 &= ~0x07;
		LPC_IOCON->PIO0_9 |= 0x01; /* SSP MOSI */

		LPC_IOCON->SCK_LOC = 0x02;
		LPC_IOCON->PIO0_6 = 0x02; /* P0.6 function 2 is SSP clock, need to
		 combined with IOCONSCKLOC register setting */

		LPC_IOCON->PIO0_2 &= ~0x07;
		LPC_IOCON->PIO0_2 |= 0x01; /* SSP SSEL */

		/* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
		LPC_SSP0->CR0 = 0x0707
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
				;

		/* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
		LPC_SSP0->CPSR = 0x2;

		for (i = 0; i < FIFOSIZE; i++)
		{
			dummy = LPC_SSP0->DR; /* clear the RxFIFO */
		}
		/* Device select as master, SSP Enabled */

		/* Master mode */
		LPC_SSP0->CR1 = SSPCR1_SSE;

		/* Set SSPINMS registers to enable interrupts */
		/* enable all error related interrupts */
		LPC_SSP0->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;

		/* Enable the SSP Interrupt */
		NVIC_EnableIRQ(SSP0_IRQn);
	}
}

void ssp_send(uint8_t port, const uint8_t* buf, uint32_t length)
{
	volatile uint32_t i = 0;
	volatile uint8_t Dummy = Dummy;

	if (port == 0)
	{
		for (i = 0; i < length; i++)
		{
			/* Move on only if NOT busy and TX FIFO not full. */
			while ((LPC_SSP0->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
				;
			LPC_SSP0->DR = *buf;
			buf++;
#if !LOOPBACK_MODE
			while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
			/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
			 on MISO. Otherwise, when SSP0Receive() is called, previous data byte
			 is left in the FIFO. */
			Dummy = LPC_SSP0->DR;
#else
			/* Wait until the Busy bit is cleared. */
			while ( LPC_SSP0->SR & SSPSR_BSY );
#endif
		}
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			/* Move on only if NOT busy and TX FIFO not full. */
			while ((LPC_SSP1->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
				;
			LPC_SSP1->DR = *buf;
			buf++;
#if !LOOPBACK_MODE
			while ((LPC_SSP1->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
			/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
			 on MISO. Otherwise, when SSP0Receive() is called, previous data byte
			 is left in the FIFO. */
			Dummy = LPC_SSP1->DR;
#else
			/* Wait until the Busy bit is cleared. */
			while ( LPC_SSP1->SR & SSPSR_BSY );
#endif
		}
	}
}

void ssp_send_uint16(uint8_t port, const uint16_t* buf, uint32_t length)
{
	volatile uint32_t i = 0;
	volatile uint16_t dummy = dummy;

	if (port == 0)
	{
		for (i = 0; i < length; i++)
		{
			/* Move on only if NOT busy and TX FIFO not full. */
			while ((LPC_SSP0->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
				;
			LPC_SSP0->DR = *buf;
			buf++;
#if !LOOPBACK_MODE
			while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
			/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
			 on MISO. Otherwise, when SSP0Receive() is called, previous data byte
			 is left in the FIFO. */
			dummy = LPC_SSP0->DR;
#else
			/* Wait until the Busy bit is cleared. */
			while ( LPC_SSP0->SR & SSPSR_BSY );
#endif
		}
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			/* Move on only if NOT busy and TX FIFO not full. */
			while ((LPC_SSP1->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
				;
			LPC_SSP1->DR = *buf;
			buf++;
#if !LOOPBACK_MODE
			while ((LPC_SSP1->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
			/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
			 on MISO. Otherwise, when SSP0Receive() is called, previous data byte
			 is left in the FIFO. */
			dummy = LPC_SSP1->DR;
#else
			/* Wait until the Busy bit is cleared. */
			while ( LPC_SSP1->SR & SSPSR_BSY );
#endif
		}
	}
}

void ssp_receive(uint8_t port, uint8_t* buf, uint32_t length)
{
	volatile uint32_t i;

	/* As long as Receive FIFO is not empty, I can always receive. */
	/* If it's a loopback test, clock is shared for both TX and RX,
	 no need to write dummy byte to get clock to get the data */
	/* if it's a peer-to-peer communication, SSPDR needs to be written
	 before a read can take place. */
	if (port == 0)
	{
		for (i = 0; i < length; i++)
		{
#if !LOOPBACK_MODE
#if SSP0_SLAVE
			while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#else
			LPC_SSP0->DR = 0xFF;
			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
#endif
#else
			while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#endif
			*buf = LPC_SSP0->DR;
			buf++;
		}
	}
	else
	{
		for (i = 0; i < length; i++)
		{
#if !LOOPBACK_MODE
#if SSP1_SLAVE
			while (!(LPC_SSP1->SR & SSPSR_RNE))
				;
#else
			LPC_SSP1->DR = 0xFF;
			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP1->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
			;
#endif
#else
			while ( !(LPC_SSP1->SR & SSPSR_RNE) );
#endif
			*buf = LPC_SSP1->DR;
			buf++;
		}
	}
}

void ssp_receive_uint16(uint8_t port, uint16_t* buf, uint32_t length)
{
	volatile uint32_t i;

	/* As long as Receive FIFO is not empty, I can always receive. */
	/* If it's a loopback test, clock is shared for both TX and RX,
	 no need to write dummy byte to get clock to get the data */
	/* if it's a peer-to-peer communication, SSPDR needs to be written
	 before a read can take place. */
	if (port == 0)
	{
		for (i = 0; i < length; i++)
		{
#if !LOOPBACK_MODE
#if SSP0_SLAVE
			while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#else
			LPC_SSP0->DR = 0xFFFF;
			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
#endif
#else
			while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#endif
			*buf = LPC_SSP0->DR;
			buf++;
		}
	}
	else
	{
		for (i = 0; i < length; i++)
		{
#if !LOOPBACK_MODE
#if SSP1_SLAVE
			while (!(LPC_SSP1->SR & SSPSR_RNE))
				;
#else
			LPC_SSP1->DR = 0xFFFF;
			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP1->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
			;
#endif
#else
			while ( !(LPC_SSP1->SR & SSPSR_RNE) );
#endif
			*buf = LPC_SSP1->DR;
			buf++;
		}
	}
}

void ssp_exchange(uint8_t port, uint8_t* buf, uint32_t length)
{
	volatile uint32_t i;

	for (i = 0; i < length; i++)
	{
		/* As long as Receive FIFO is not empty, I can always receive. */
		/* If it's a loopback test, clock is shared for both TX and RX,
		 no need to write dummy byte to get clock to get the data */
		/* if it's a peer-to-peer communication, SSPDR needs to be written
		 before a read can take place. */
		if (port == 0)
		{
#if !LOOPBACK_MODE
#if SSP0_SLAVE
			while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#else
			LPC_SSP0->DR = *buf;
			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP0->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
				;
#endif
#else
			while ( !(LPC_SSP0->SR & SSPSR_RNE) );
#endif
			*buf = LPC_SSP0->DR;
			buf++;
		}
		else
		{
#if !LOOPBACK_MODE
#if SSP1_SLAVE
			while (!(LPC_SSP1->SR & SSPSR_RNE))
				;
#else
			LPC_SSP1->DR = *buf;
			/* Wait until the Busy bit is cleared */
			while ((LPC_SSP1->SR & (SSPSR_BSY | SSPSR_RNE)) != SSPSR_RNE)
			;
#endif
#else
			while ( !(LPC_SSP1->SR & SSPSR_RNE) );
#endif
			*buf = LPC_SSP1->DR;
			buf++;
		}
	}
}

void SSP0_IRQHandler(void)
{
	volatile uint32_t regValue;

	regValue = LPC_SSP0->MIS;
	if (regValue & SSPMIS_RORMIS) /* Receive overrun interrupt */
	{
		LPC_SSP0->ICR = SSPICR_RORIC; /* clear interrupt */
	}
	if (regValue & SSPMIS_RTMIS) /* Receive timeout interrupt */
	{
		LPC_SSP0->ICR = SSPICR_RTIC; /* clear interrupt */
	}

	/* please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
	 are shared as global variables. It may create some race condition that main
	 and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
	 in both main and ISR could prevent this kind of race condition */
	if (regValue & SSPMIS_RXMIS) /* Rx at least half full */
	{
	}
	return;
}

static uint32_t counter = 0;

void SSP1_IRQHandler(void)
{
	volatile uint32_t regValue;

	long xHigherPriorityTaskWoken = pdFALSE;

	regValue = LPC_SSP1->MIS;
	if (regValue & SSPMIS_RORMIS) /* Receive overrun interrupt */
	{
		LPC_SSP1->ICR = SSPICR_RORIC; /* clear interrupt */
	}
	if (regValue & SSPMIS_RTMIS) /* Receive timeout interrupt */
	{
		LPC_SSP1->ICR = SSPICR_RTIC; /* clear interrupt */
	}
	if (regValue & SSPMIS_RXMIS) /* Rx at least half full */
	{
		while ((LPC_SSP1->SR & SSPSR_RNE) == SSPSR_RNE)
		{
			while (LPC_SSP1->SR & SSPSR_BSY)
				;
			regValue = LPC_SSP1->DR;

			if (!xQueueIsQueueFullFromISR(xSSP1RxQueue))
			{
				/* Post the byte. */
				xQueueSendFromISR(xSSP1RxQueue, &regValue,
						&xHigherPriorityTaskWoken);
			}
		}
	}
	if (regValue & SSPMIS_TXMIS) /* Tx at least half empty */
	{
		while ((LPC_SSP1->SR & SSPSR_TNF) == SSPSR_TNF
				&& xQueuePeekFromISR(xSSP1TxQueue, &regValue) == pdTRUE)
		{
			while (LPC_SSP1->SR & SSPSR_BSY)
				;
			LPC_SSP1->DR = regValue;

			xQueueReceiveFromISR(xSSP1TxQueue, &regValue,
					&xHigherPriorityTaskWoken);
		}
	}

	/* Clear the interrupt if necessary. */
	//Dummy_ClearITPendingBit();
	/* This interrupt does nothing more than demonstrate how to synchronise a
	 task with an interrupt.  A semaphore is used for this purpose.  Note
	 lHigherPriorityTaskWoken is initialised to zero.  Only FreeRTOS API functions
	 that end in "FromISR" can be called from an ISR. */
	//xSemaphoreGiveFromISR( xTestSemaphore, &lHigherPriorityTaskWoken );
	/* If there was a task that was blocked on the semaphore, and giving the
	 semaphore caused the task to unblock, and the unblocked task has a priority
	 higher than the current Running state task (the task that this interrupt
	 interrupted), then lHigherPriorityTaskWoken will have been set to pdTRUE
	 internally within xSemaphoreGiveFromISR().  Passing pdTRUE into the
	 portEND_SWITCHING_ISR() macro will result in a context switch being pended to
	 ensure this interrupt returns directly to the unblocked, higher priority,
	 task.  Passing pdFALSE into portEND_SWITCHING_ISR() has no effect. */
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

	return;
}

