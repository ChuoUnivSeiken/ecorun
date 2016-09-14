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

#include "../system/cmsis/LPC13Uxx.h"
#include "../system/peripheral/usart.h"
#include "usart_handler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define USE_QUEUE 0

static xQueueHandle usart_rx_queue = NULL;

static void usart_receive_data(uint8_t data);

/* Priorities at which the tasks are created. */
#define USART_RECEIVE_DATA_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define USART_RECEIVE_DATA_FREQENCY_MS	( 20 / portTICK_RATE_MS )

static void usart_receive_data_task(void* parameters)
{
	portTickType xNextWakeTime;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();
	for (;;)
	{
		vTaskDelayUntil(&xNextWakeTime, USART_RECEIVE_DATA_FREQENCY_MS);

		uint8_t data;
		while (xQueueReceive(usart_rx_queue, &data, 0) == pdTRUE)
		{
			usart_receive_data(data);
		}
	}
}

static inline void write_error(const_string msg, portBASE_TYPE code)
{
	usart_write_string("msg <");
	usart_write_string(msg);
	usart_write_string(", code ");
	usart_write_int32((int) code);
	usart_writeln_string(">");
}

void usart_handler_init(void)
{
#if USE_QUEUE
	usart_rx_queue = xQueueCreate(1024, sizeof(uint8_t));

	if (usart_rx_queue == NULL)
	{
		usart_write_string("msg <can't allocate usart_rx_queue.");
		usart_writeln_string(">");
	}

	portBASE_TYPE result;

	if ((result = xTaskCreate(usart_receive_data_task, "usart_rx_data",
			configMINIMAL_STACK_SIZE, NULL, USART_RECEIVE_DATA_TASK_PRIORITY,
			NULL)) != pdPASS)
	{
		write_error("cannot create \"usart_rx_data\" task.", result);
	}
#endif
}

volatile uint8_t usart_buf_arr[USART_BUFFER_SIZE];
volatile uint8_t* usart_buf_ptr = usart_buf_arr;
volatile uint32_t usart_buf_count = 0;

#define WEAK __attribute__((weak))

WEAK void usart_receive_data_handler(string str, uint32_t str_len);

static void usart_receive_data(uint8_t data)
{
	if (usart_buf_count < USART_BUFFER_SIZE)
	{
		switch (data)
		{
		case 0x05:
			usart_write_char(0x06);
			break;
		case 0x02:
			usart_buf_count = 0;
			break;
		case 0x03:
			usart_buf_arr[usart_buf_count] = '\0';
			usart_receive_data_handler(usart_buf_arr, usart_buf_count);
			break;
		default:
			usart_buf_arr[usart_buf_count++] = data;
		}
	}
	else
	{
		usart_buf_count = 0; // buffer overflow
		usart_write_string("uart buffer overflow.");
	}
}

void USART_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;

	IIRValue = LPC_USART->IIR;
	IIRValue &= ~(USART_IIR_IntStatus_MASK); /* skip pending bit in IIR */
	IIRValue &= USART_IIR_IntId_MASK; /* check bit 1~3, interrupt identification */

	// 1.) Check receiver line status
	if (IIRValue == USART_IIR_IntId_RLS)
	{
		LSRValue = LPC_USART->LSR;
		// Check for errors
		if (LSRValue
				& (USART_LSR_OE | USART_LSR_PE | USART_LSR_FE | USART_LSR_RXFE
						| USART_LSR_BI))
		{
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			//uart_pcb.status = LSRValue;
			Dummy = LPC_USART->RBR; /* Dummy read on RX to clear interrupt, then bail out */
			return;
		}
		// No error and receive data is ready
		if (LSRValue & USART_LSR_RDR_DATA)
		{
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			usart_buf_arr[usart_buf_count++] = LPC_USART->RBR;
			if (usart_buf_count == USART_BUFFER_SIZE)
			{
				usart_buf_count = 0; /* buffer overflow */
			}
		}
	}

	// 2.) Check receive data available
	else if (IIRValue == USART_IIR_IntId_RDA)
	{
		// Add incoming text to UART buffer
		uint8_t data = LPC_USART->RBR;

#if USE_QUEUE
		if (usart_rx_queue)
		{
			xQueueSendToBackFromISR(usart_rx_queue, &data,
					&xHigherPriorityTaskWoken);
		}
#else
		usart_receive_data(data);
#endif
	}

	// 3.) Check character timeout indicator
	else if (IIRValue == USART_IIR_IntId_CTI)
	{
		/* Bit 9 as the CTI error */
		//uart_pcb.status |= 0x100;
	}

	// 4.) Check THRE (transmit holding register empty)
	else if (IIRValue == USART_IIR_IntId_THRE)
	{
		/* Check status in the LSR to see if valid data in U0THR or not */
		LSRValue = LPC_USART->LSR;
		if (LSRValue & USART_LSR_THRE)
		{
			//uart_pcb.pending_tx_data = 0;
		}
		else
		{
			//uart_pcb.pending_tx_data = 1;
		}
	}

	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	return;
}
