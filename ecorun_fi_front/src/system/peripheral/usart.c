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
 * �\�[�X�R�[�h�`�����o�C�i���`�����A�ύX���邩���Ȃ������킸�A�ȉ��̏����𖞂����ꍇ�Ɍ���A�ĔЕz����юg�p��������܂��B
 *
 * �\�[�X�R�[�h���ĔЕz����ꍇ�A��L�̒��쌠�\���A�{�����ꗗ�A����щ��L�Ɛӏ������܂߂邱�ƁB
 * �o�C�i���`���ōĔЕz����ꍇ�A�Еz���ɕt���̃h�L�������g���̎����ɁA��L�̒��쌠�\���A�{�����ꗗ�A����щ��L�Ɛӏ������܂߂邱�ƁB
 * ���ʂɂ����ʂ̋��Ȃ��ɁA�{�\�t�g�E�F�A����h���������i�̐�`�܂��͔̔����i�ɁA<�g�D>�̖��O�܂��̓R���g���r���[�^�[�̖��O���g�p���Ă͂Ȃ�Ȃ��B
 * �{�\�t�g�E�F�A�́A���쌠�҂���уR���g���r���[�^�[�ɂ���āu����̂܂܁v�񋟂���Ă���A�����َ����킸�A���ƓI�Ȏg�p�\���A����ѓ���̖ړI�ɑ΂���K�����Ɋւ���Öق̕ۏ؂��܂߁A�܂�����Ɍ��肳��Ȃ��A�����Ȃ�ۏ؂�����܂���B���쌠�҂��R���g���r���[�^�[���A���R�̂�������킸�A ���Q�����̌�����������킸�A���ӔC�̍������_��ł��邩���i�ӔC�ł��邩�i�ߎ����̑��́j�s�@�s�ׂł��邩���킸�A���ɂ��̂悤�ȑ��Q����������\����m�炳��Ă����Ƃ��Ă��A�{�\�t�g�E�F�A�̎g�p�ɂ���Ĕ��������i��֕i�܂��͑�p�T�[�r�X�̒��B�A�g�p�̑r���A�f�[�^�̑r���A���v�̑r���A�Ɩ��̒��f���܂߁A�܂�����Ɍ��肳��Ȃ��j���ڑ��Q�A�Ԑڑ��Q�A�����I�ȑ��Q�A���ʑ��Q�A�����I���Q�A�܂��͌��ʑ��Q�ɂ��āA��ؐӔC�𕉂�Ȃ����̂Ƃ��܂��B
 */

#include "../cmsis/LPC13Uxx.h"
#include "usart.h"

static uint32_t is_start_of_txt = 0;

void usart_init(uint32_t baudrate)
{
	volatile uint32_t fdiv, regVal;

	/* Enable UART clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12);
	LPC_SYSCON->UARTCLKDIV = 0x1;

	/* Set DLAB back to 0 */
	LPC_USART->LCR = (USART_LCR_Word_Length_Select_8Chars |
	USART_LCR_Stop_Bit_Select_1Bits |
	USART_LCR_Parity_Disabled |
	USART_LCR_Parity_Select_OddParity |
	USART_LCR_Break_Control_Disabled |
	USART_LCR_Divisor_Latch_Access_Enabled);

	/* Baud rate */
	regVal = LPC_SYSCON->UARTCLKDIV;
	fdiv = ((SystemCoreClock / LPC_SYSCON->UARTCLKDIV) / 16) / baudrate;

	LPC_USART->DLM = fdiv / 256;
	LPC_USART->DLL = fdiv % 256;

	/* Set DLAB back to 0 */
	LPC_USART->LCR = (USART_LCR_Word_Length_Select_8Chars |
	USART_LCR_Stop_Bit_Select_1Bits |
	USART_LCR_Parity_Disabled |
	USART_LCR_Parity_Select_OddParity |
	USART_LCR_Break_Control_Disabled |
	USART_LCR_Divisor_Latch_Access_Disabled);

	/* Enable and reset TX and RX FIFO. */
	LPC_USART->FCR = (USART_FCR_FIFO_Enabled |
	USART_FCR_Rx_FIFO_Reset |
	USART_FCR_Tx_FIFO_Reset);

	/* Read to clear the line status. */
	regVal = LPC_USART->LSR;

	/* Ensure a clean start, no data in either TX or RX FIFO. */
	while (( LPC_USART->LSR & (USART_LSR_THRE | USART_LSR_TEMT))
			!= (USART_LSR_THRE | USART_LSR_TEMT))
		;
	while ( LPC_USART->LSR & USART_LSR_RDR_DATA)
	{
		/* Dump data from RX FIFO */
		regVal = LPC_USART->RBR;
	}

	NVIC_EnableIRQ(USART_IRQn);
	LPC_USART->IER = USART_IER_RBR_Interrupt_Enabled
			| USART_IER_RLS_Interrupt_Enabled;

	is_start_of_txt = 1;
}

void usart_write_char(uint8_t c)
{
	if (is_start_of_txt)
	{
		is_start_of_txt = 0;
		usart_write_char(USART_STX);
	}
	if (c == USART_ETX)
	{
		is_start_of_txt = 1;
	}
	while (!(LPC_USART->LSR & USART_LSR_THRE))
	{
		// no operation
	}
	LPC_USART->THR = c;
}

uint32_t usart_writeln_string(const_string s)
{
	volatile uint32_t n;
	n = usart_write_string(s);
	usart_endln();
	return n;
}

uint32_t usart_write_string(const_string s)
{
	volatile uint32_t n;
	for (n = 0; s[n] != '\0'; n++)
	{
		usart_write_char(s[n]);
	}
	return n;
}

void usart_endln(void)
{
	usart_write_char(USART_ETX);
}

