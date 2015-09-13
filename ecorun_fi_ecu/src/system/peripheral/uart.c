/*
 * uart.c
 *
 *  Created on: 2014/06/05
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "uart.h"

/*  Line control */
#define UART_LCR_Word_Length_Select_MASK        ((unsigned int) 0x00000003) // Word Length Selector
#define UART_LCR_Word_Length_Select_5Chars      ((unsigned int) 0x00000000)
#define UART_LCR_Word_Length_Select_6Chars      ((unsigned int) 0x00000001)
#define UART_LCR_Word_Length_Select_7Chars      ((unsigned int) 0x00000002)
#define UART_LCR_Word_Length_Select_8Chars      ((unsigned int) 0x00000003)
#define UART_LCR_Stop_Bit_Select_MASK           ((unsigned int) 0x00000004) // Stop bit select
#define UART_LCR_Stop_Bit_Select_1Bits          ((unsigned int) 0x00000000)
#define UART_LCR_Stop_Bit_Select_2Bits          ((unsigned int) 0x00000004)
#define UART_LCR_Parity_Enable_MASK             ((unsigned int) 0x00000008) // Parity enable
#define UART_LCR_Parity_Enabled                 ((unsigned int) 0x00000008)
#define UART_LCR_Parity_Disabled                ((unsigned int) 0x00000000)
#define UART_LCR_Parity_Select_MASK             ((unsigned int) 0x00000030) // Parity select
#define UART_LCR_Parity_Select_OddParity        ((unsigned int) 0x00000000)
#define UART_LCR_Parity_Select_EvenParity       ((unsigned int) 0x00000010)
#define UART_LCR_Parity_Select_Forced1          ((unsigned int) 0x00000020)
#define UART_LCR_Parity_Select_Forced0          ((unsigned int) 0x00000030)
#define UART_LCR_Break_Control_MASK             ((unsigned int) 0x00000040) // Break transmission control
#define UART_LCR_Break_Control_Enabled          ((unsigned int) 0x00000040)
#define UART_LCR_Break_Control_Disabled         ((unsigned int) 0x00000000)
#define UART_LCR_Divisor_Latch_Access_MASK      ((unsigned int) 0x00000080) // Divisor latch access
#define UART_LCR_Divisor_Latch_Access_Enabled   ((unsigned int) 0x00000080)
#define UART_LCR_Divisor_Latch_Access_Disabled  ((unsigned int) 0x00000000)

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

/*  FIFO control */
#define UART_FCR_FIFO_Enable_MASK               ((unsigned int) 0x00000001) // UART FIFOs enabled/disabled
#define UART_FCR_FIFO_Enabled                   ((unsigned int) 0x00000001)
#define UART_FCR_FIFO_Disabled                  ((unsigned int) 0x00000000)
#define UART_FCR_Rx_FIFO_Reset_MASK             ((unsigned int) 0x00000002)
#define UART_FCR_Rx_FIFO_Reset                  ((unsigned int) 0x00000002) // Clear Rx FIFO
#define UART_FCR_Tx_FIFO_Reset_MASK             ((unsigned int) 0x00000004)
#define UART_FCR_Tx_FIFO_Reset                  ((unsigned int) 0x00000004) // Clear Tx FIFO
#define UART_FCR_Rx_Trigger_Level_Select_MASK   ((unsigned int) 0x000000C0) // Chars written before before interrupt
#define UART_FCR_Rx_Trigger_Level_Select_1Char  ((unsigned int) 0x00000000)
#define UART_FCR_Rx_Trigger_Level_Select_4Char  ((unsigned int) 0x00000040)
#define UART_FCR_Rx_Trigger_Level_Select_8Char  ((unsigned int) 0x00000080)
#define UART_FCR_Rx_Trigger_Level_Select_12Char ((unsigned int) 0x000000C0)

/* Interrupt enable */
#define UART_IER_RBR_Interrupt_MASK             ((unsigned int) 0x00000001) // Enables the received data available interrupt
#define UART_IER_RBR_Interrupt_Enabled          ((unsigned int) 0x00000001)
#define UART_IER_RBR_Interrupt_Disabled         ((unsigned int) 0x00000000)
#define UART_IER_THRE_Interrupt_MASK            ((unsigned int) 0x00000002) // Enables the THRE interrupt
#define UART_IER_THRE_Interrupt_Enabled         ((unsigned int) 0x00000002)
#define UART_IER_THRE_Interrupt_Disabled        ((unsigned int) 0x00000000)
#define UART_IER_RLS_Interrupt_MASK             ((unsigned int) 0x00000004) // Enables the Rx line status interrupt
#define UART_IER_RLS_Interrupt_Enabled          ((unsigned int) 0x00000004)
#define UART_IER_RLS_Interrupt_Disabled         ((unsigned int) 0x00000000)
#define UART_IER_ABEOIntEn_MASK                 ((unsigned int) 0x00000100) // End of auto-baud interrupt
#define UART_IER_ABEOIntEn_Enabled              ((unsigned int) 0x00000100)
#define UART_IER_ABEOIntEn_Disabled             ((unsigned int) 0x00000000)
#define UART_IER_ABTOIntEn_MASK                 ((unsigned int) 0x00000200) // Auto-baud timeout interrupt
#define UART_IER_ABTOIntEn_Enabled              ((unsigned int) 0x00000200)
#define UART_IER_ABTOIntEn_Disabled             ((unsigned int) 0x00000000)

static uint32_t is_start_of_txt = 0;

void uart_init(uint32_t baudrate)
{
	volatile uint32_t fdiv, regVal;

	NVIC_DisableIRQ(UART_IRQn);

	// configure P1[7] as TXD output
	LPC_IOCON->PIO1_7 &= ~0x07;
	LPC_IOCON->PIO1_7 |= 0x01;

	LPC_IOCON->PIO1_7 = 0b01;

	// configure P1[7] as TXD output
	LPC_IOCON->PIO1_6 &= ~0x07;
	LPC_IOCON->PIO1_6 |= 0x01;

	LPC_IOCON->PIO1_6 = 0b01;

	// enable uart clock
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12); // UART=1

	// enable uart peripheral clock
	LPC_SYSCON->UARTCLKDIV = 0x01; // div=1

	/* 8 bits, no Parity, 1 Stop bit */
	LPC_UART->LCR = (UART_LCR_Word_Length_Select_8Chars |
	UART_LCR_Stop_Bit_Select_1Bits |
	UART_LCR_Parity_Disabled |
	UART_LCR_Parity_Select_OddParity |
	UART_LCR_Break_Control_Disabled |
	UART_LCR_Divisor_Latch_Access_Enabled);

	/* Baud rate */
	regVal = LPC_SYSCON->UARTCLKDIV;
	fdiv = (SystemCoreClock // System clock frequency
	* LPC_SYSCON->SYSAHBCLKDIV // AHB clock divider
	) / (regVal // UART clock divider
	* 16 * baudrate // baud rate clock
	);

	LPC_UART->DLM = fdiv / 256;
	LPC_UART->DLL = fdiv % 256;

	/* Set DLAB back to 0 */
	LPC_UART->LCR = (UART_LCR_Word_Length_Select_8Chars |
	UART_LCR_Stop_Bit_Select_1Bits |
	UART_LCR_Parity_Disabled |
	UART_LCR_Parity_Select_OddParity |
	UART_LCR_Break_Control_Disabled |
	UART_LCR_Divisor_Latch_Access_Disabled);

	LPC_UART->FCR = (UART_FCR_FIFO_Enabled | UART_FCR_Rx_FIFO_Reset
			| UART_FCR_Tx_FIFO_Reset);

	/* Read to clear the line status. */
	regVal = LPC_UART->LSR;

	while (( LPC_UART->LSR & (UART_LSR_THRE | UART_LSR_TEMT))
			!= (UART_LSR_THRE | UART_LSR_TEMT))
		;
	while ( LPC_UART->LSR & UART_LSR_RDR_DATA)
	{
		/* Dump data from RX FIFO */
		regVal = LPC_UART->RBR;
	}

	NVIC_EnableIRQ(UART_IRQn);
	LPC_UART->IER = UART_IER_RBR_Interrupt_Enabled
			| UART_IER_RLS_Interrupt_Enabled;

	is_start_of_txt = 1;
}

void uart_write_char(uint8_t c)
{
	if (is_start_of_txt)
	{
		is_start_of_txt = 0;
		uart_write_char(UART_STX);
	}
	if (c == UART_ETX)
	{
		is_start_of_txt = 1;
	}
	// wait for tx buffer empty
	while (!(LPC_UART->LSR & UART_LSR_THRE))
		;
	// put a character
	LPC_UART->THR = c;
}

uint32_t uart_writeln_string(const uint8_t* s)
{
	volatile uint32_t n;
	n = uart_write_string(s);
	uart_endln();
	return n;
}

uint32_t uart_write_string(const uint8_t* s)
{
	volatile uint32_t n;
	for (n = 0; s[n] != '\0'; n++)
	{
		uart_write_char(s[n]);
	}
	return n;
}

void uart_endln(void)
{
	uart_write_char(UART_ETX);
}

