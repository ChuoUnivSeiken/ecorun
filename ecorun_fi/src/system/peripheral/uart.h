/*
 * Copyright (c) 2013, K. Townsend (microBuilder.eu)
 * https://github.com/microbuilder/
 * All rights reserved.
 *
 * Modified by Yoshio Nakamura, Copyright (c) 2015
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ソースコード形式かバイナリ形式か、変更するかしないかを問わず、以下の条件を満たす場合に限り、再頒布および使用が許可されます。
 *
 * ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、および下記免責条項を含めること。
 * バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、上記の著作権表示、本条件一覧、および下記免責条項を含めること。
 * 書面による特別の許可なしに、本ソフトウェアから派生した製品の宣伝または販売促進に、<組織>の名前またはコントリビューターの名前を使用してはならない。
 * 本ソフトウェアは、著作権者およびコントリビューターによって「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、および特定の目的に対する適合性に関する暗黙の保証も含め、またそれに限定されない、いかなる保証もありません。著作権者もコントリビューターも、事由のいかんを問わず、 損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか（過失その他の）不法行為であるかを問わず、仮にそのような損害が発生する可能性を知らされていたとしても、本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、使用の喪失、データの喪失、利益の喪失、業務の中断も含め、またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

#define TX_INTERRUPT 0

#define USART_IER_RBR_Interrupt_MASK             ((unsigned int) 0x00000001) // Enables the received data available interrupt#define USART_IER_RBR_Interrupt_Enabled          ((unsigned int) 0x00000001)#define USART_IER_RBR_Interrupt_Disabled         ((unsigned int) 0x00000000)#define USART_IER_THRE_Interrupt_MASK            ((unsigned int) 0x00000002) // Enables the THRE interrupt#define USART_IER_THRE_Interrupt_Enabled         ((unsigned int) 0x00000002)#define USART_IER_THRE_Interrupt_Disabled        ((unsigned int) 0x00000000)#define USART_IER_RLS_Interrupt_MASK             ((unsigned int) 0x00000004) // Enables the Rx line status interrupt#define USART_IER_RLS_Interrupt_Enabled          ((unsigned int) 0x00000004)#define USART_IER_RLS_Interrupt_Disabled         ((unsigned int) 0x00000000)#define USART_IER_ABEOIntEn_MASK                 ((unsigned int) 0x00000100) // End of auto-baud interrupt#define USART_IER_ABEOIntEn_Enabled              ((unsigned int) 0x00000100)#define USART_IER_ABEOIntEn_Disabled             ((unsigned int) 0x00000000)#define USART_IER_ABTOIntEn_MASK                 ((unsigned int) 0x00000200) // Auto-baud timeout interrupt#define USART_IER_ABTOIntEn_Enabled              ((unsigned int) 0x00000200)#define USART_IER_ABTOIntEn_Disabled             ((unsigned int) 0x00000000)//
#define USART_IIR_IntStatus_MASK                 ((unsigned int) 0x00000001) // Interrupt status#define USART_IIR_IntStatus_InterruptPending     ((unsigned int) 0x00000001)#define USART_IIR_IntStatus_NoInterruptPending   ((unsigned int) 0x00000000)#define USART_IIR_IntId_MASK                     ((unsigned int) 0x0000000E) // Interrupt identification#define USART_IIR_IntId_RLS                      ((unsigned int) 0x00000006) // Receive line status#define USART_IIR_IntId_RDA                      ((unsigned int) 0x00000004) // Receive data available#define USART_IIR_IntId_CTI                      ((unsigned int) 0x0000000C) // Character time-out indicator#define USART_IIR_IntId_THRE                     ((unsigned int) 0x00000002) // THRE interrupt#define USART_IIR_IntId_MODEM                    ((unsigned int) 0x00000000) // Modem interrupt#define USART_IIR_FIFO_Enable_MASK               ((unsigned int) 0x000000C0)#define USART_IIR_ABEOInt_MASK                   ((unsigned int) 0x00000100) // End of auto-baud interrupt#define USART_IIR_ABEOInt                        ((unsigned int) 0x00000100)#define USART_IIR_ABTOInt_MASK                   ((unsigned int) 0x00000200) // Auto-baud time-out interrupt#define USART_IIR_ABTOInt                        ((unsigned int) 0x00000200)//

#define USART_FCR_FIFO_Enable_MASK               ((unsigned int) 0x00000001) // UART FIFOs enabled/disabled#define USART_FCR_FIFO_Enabled                   ((unsigned int) 0x00000001)#define USART_FCR_FIFO_Disabled                  ((unsigned int) 0x00000000)#define USART_FCR_Rx_FIFO_Reset_MASK             ((unsigned int) 0x00000002)#define USART_FCR_Rx_FIFO_Reset                  ((unsigned int) 0x00000002) // Clear Rx FIFO#define USART_FCR_Tx_FIFO_Reset_MASK             ((unsigned int) 0x00000004)#define USART_FCR_Tx_FIFO_Reset                  ((unsigned int) 0x00000004) // Clear Tx FIFO#define USART_FCR_Rx_Trigger_Level_Select_MASK   ((unsigned int) 0x000000C0) // Chars written before before interrupt#define USART_FCR_Rx_Trigger_Level_Select_1Char  ((unsigned int) 0x00000000)#define USART_FCR_Rx_Trigger_Level_Select_4Char  ((unsigned int) 0x00000040)#define USART_FCR_Rx_Trigger_Level_Select_8Char  ((unsigned int) 0x00000080)#define USART_FCR_Rx_Trigger_Level_Select_12Char ((unsigned int) 0x000000C0)#define USART_LCR_Word_Length_Select_MASK        ((unsigned int) 0x00000003) // Word Length Selector#define USART_LCR_Word_Length_Select_5Chars      ((unsigned int) 0x00000000)#define USART_LCR_Word_Length_Select_6Chars      ((unsigned int) 0x00000001)#define USART_LCR_Word_Length_Select_7Chars      ((unsigned int) 0x00000002)#define USART_LCR_Word_Length_Select_8Chars      ((unsigned int) 0x00000003)#define USART_LCR_Stop_Bit_Select_MASK           ((unsigned int) 0x00000004) // Stop bit select#define USART_LCR_Stop_Bit_Select_1Bits          ((unsigned int) 0x00000000)#define USART_LCR_Stop_Bit_Select_2Bits          ((unsigned int) 0x00000004)#define USART_LCR_Parity_Enable_MASK             ((unsigned int) 0x00000008) // Parity enable#define USART_LCR_Parity_Enabled                 ((unsigned int) 0x00000008)#define USART_LCR_Parity_Disabled                ((unsigned int) 0x00000000)#define USART_LCR_Parity_Select_MASK             ((unsigned int) 0x00000030) // Parity select#define USART_LCR_Parity_Select_OddParity        ((unsigned int) 0x00000000)#define USART_LCR_Parity_Select_EvenParity       ((unsigned int) 0x00000010)#define USART_LCR_Parity_Select_Forced1          ((unsigned int) 0x00000020)#define USART_LCR_Parity_Select_Forced0          ((unsigned int) 0x00000030)#define USART_LCR_Break_Control_MASK             ((unsigned int) 0x00000040) // Break transmission control#define USART_LCR_Break_Control_Enabled          ((unsigned int) 0x00000040)#define USART_LCR_Break_Control_Disabled         ((unsigned int) 0x00000000)#define USART_LCR_Divisor_Latch_Access_MASK      ((unsigned int) 0x00000080) // Divisor latch access#define USART_LCR_Divisor_Latch_Access_Enabled   ((unsigned int) 0x00000080)#define USART_LCR_Divisor_Latch_Access_Disabled  ((unsigned int) 0x00000000)#define USART_LSR_RDR_MASK                       ((unsigned int) 0x00000001) // Receiver data ready#define USART_LSR_RDR_EMPTY                      ((unsigned int) 0x00000000) // U0RBR is empty#define USART_LSR_RDR_DATA                       ((unsigned int) 0x00000001) // U0RBR contains valid data#define USART_LSR_OE_MASK                        ((unsigned int) 0x00000002) // Overrun error#define USART_LSR_OE                             ((unsigned int) 0x00000002)#define USART_LSR_PE_MASK                        ((unsigned int) 0x00000004) // Parity error#define USART_LSR_PE                             ((unsigned int) 0x00000004)#define USART_LSR_FE_MASK                        ((unsigned int) 0x00000008) // Framing error#define USART_LSR_FE                             ((unsigned int) 0x00000008)#define USART_LSR_BI_MASK                        ((unsigned int) 0x00000010) // Break interrupt#define USART_LSR_BI                             ((unsigned int) 0x00000010)#define USART_LSR_THRE_MASK                      ((unsigned int) 0x00000020) // Transmitter holding register empty#define USART_LSR_THRE                           ((unsigned int) 0x00000020)#define USART_LSR_TEMT_MASK                      ((unsigned int) 0x00000040) // Transmitter empty#define USART_LSR_TEMT                           ((unsigned int) 0x00000040)#define USART_LSR_RXFE_MASK                      ((unsigned int) 0x00000080) // Error in Rx FIFO#define USART_LSR_RXFE                           ((unsigned int) 0x00000080)#define IER_RBR		0x01#define IER_THRE	0x02#define IER_RLS		0x04#define IIR_PEND 	0x01#define IIR_RLS		0x03
#define IIR_RDA 	0x02
#define IIR_CTI  	0x06
#define IIR_THRE  	0x01

#define LSR_RDR  	0x01
#define LSR_OE  	0x02
#define LSR_PE  	0x04
#define LSR_FE  	0x08
#define LSR_BI  	0x10
#define LSR_THRE  	0x20
#define LSR_TEMT  	0x40
#define LSR_RXFE  	0x80

#define UART_STX 0x02
#define UART_ETX 0x03

#if defined(__cplusplus)
extern "C" {
#endif

// first initalization
void uart_init(uint32_t baudrate);

// send one character
void uart_putc(uint8_t c);

// send caracters
uint32_t uart_puts(const char* s);

// send string with '\0'
int uart_puts_with_term(const char* s);

// send interger with '\0'
void uart_int32_with_term(int32_t value);
void uart_uint32_with_term(uint32_t value);
void uart_uint32_hex_with_term(uint32_t value);

void uart_int32(int32_t value);
void uart_uint32(uint32_t value);
void uart_uint32(uint32_t value);

// send UART_ETX
void uart_terminate(void);

#if defined(__cplusplus)
}
#endif

#endif /* UART_H_ */
