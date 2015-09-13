/*
 * uart_handler.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "uart.h"

/*  Interrupt identification */
#define UART_IIR_IntStatus_MASK                 ((unsigned int) 0x00000001) // Interrupt status
#define UART_IIR_IntStatus_InterruptPending     ((unsigned int) 0x00000001)
#define UART_IIR_IntStatus_NoInterruptPending   ((unsigned int) 0x00000000)
#define UART_IIR_IntId_MASK                     ((unsigned int) 0x0000000E) // Interrupt identification
#define UART_IIR_IntId_RLS                      ((unsigned int) 0x00000006) // Receive line status
#define UART_IIR_IntId_RDA                      ((unsigned int) 0x00000004) // Receive data available
#define UART_IIR_IntId_CTI                      ((unsigned int) 0x0000000C) // Character time-out indicator
#define UART_IIR_IntId_THRE                     ((unsigned int) 0x00000002) // THRE interrupt
#define UART_IIR_IntId_MODEM                    ((unsigned int) 0x00000000) // Modem interrupt
#define UART_IIR_FIFO_Enable_MASK               ((unsigned int) 0x000000C0)
#define UART_IIR_ABEOInt_MASK                   ((unsigned int) 0x00000100) // End of auto-baud interrupt
#define UART_IIR_ABEOInt                        ((unsigned int) 0x00000100)
#define UART_IIR_ABTOInt_MASK                   ((unsigned int) 0x00000200) // Auto-baud time-out interrupt
#define UART_IIR_ABTOInt                        ((unsigned int) 0x00000200)

/*  Line status */
#define UART_LSR_RDR_MASK                       ((unsigned int) 0x00000001) // Receiver data ready
#define UART_LSR_RDR_EMPTY                      ((unsigned int) 0x00000000) // U0RBR is empty
#define UART_LSR_RDR_DATA                       ((unsigned int) 0x00000001) // U0RBR contains valid data
#define UART_LSR_OE_MASK                        ((unsigned int) 0x00000002) // Overrun error
#define UART_LSR_OE                             ((unsigned int) 0x00000002)
#define UART_LSR_PE_MASK                        ((unsigned int) 0x00000004) // Parity error
#define UART_LSR_PE                             ((unsigned int) 0x00000004)
#define UART_LSR_FE_MASK                        ((unsigned int) 0x00000008) // Framing error
#define UART_LSR_FE                             ((unsigned int) 0x00000008)
#define UART_LSR_BI_MASK                        ((unsigned int) 0x00000010) // Break interrupt
#define UART_LSR_BI                             ((unsigned int) 0x00000010)
#define UART_LSR_THRE_MASK                      ((unsigned int) 0x00000020) // Transmitter holding register empty
#define UART_LSR_THRE                           ((unsigned int) 0x00000020)
#define UART_LSR_TEMT_MASK                      ((unsigned int) 0x00000040) // Transmitter empty
#define UART_LSR_TEMT                           ((unsigned int) 0x00000040)
#define UART_LSR_RXFE_MASK                      ((unsigned int) 0x00000080) // Error in Rx FIFO
#define UART_LSR_RXFE                           ((unsigned int) 0x00000080)

void UART_IRQHandler(void)
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;

	IIRValue = LPC_UART->IIR;
	IIRValue &= ~(UART_IIR_IntStatus_MASK); /* skip pending bit in IIR */
	IIRValue &= UART_IIR_IntId_MASK; /* check bit 1~3, interrupt identification */

	// 1.) Check receiver line status
	if (IIRValue == UART_IIR_IntId_RLS)
	{
		LSRValue = LPC_UART->LSR;
		// Check for errors
		if (LSRValue
				& (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE
						| UART_LSR_BI))
		{
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			Dummy = LPC_UART->RBR; /* Dummy read on RX to clear interrupt, then bail out */
			return;
		}
		// No error and receive data is ready
		if (LSRValue & UART_LSR_RDR_DATA)
		{
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
		}
	}

	// 2.) Check receive data available
	else if (IIRValue == UART_IIR_IntId_RDA)
	{
		// Add incoming text to UART buffer
	}

	// 3.) Check character timeout indicator
	else if (IIRValue == UART_IIR_IntId_CTI)
	{
		/* Bit 9 as the CTI error */
	}

	// 4.) Check THRE (transmit holding register empty)
	else if (IIRValue == UART_IIR_IntId_THRE)
	{
		/* Check status in the LSR to see if valid data in U0THR or not */
		LSRValue = LPC_UART->LSR;
		if (LSRValue & UART_LSR_THRE)
		{
		}
		else
		{
		}
	}
	return;
}

