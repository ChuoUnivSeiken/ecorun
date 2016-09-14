/*
 * spi_trasmit_data.c
 *
 *  Created on: 2015/09/04
 *      Author: Yoshio
 */

#include <string.h>

#include "spi_transmit_data.h"

#include "injection.h"
#include "../system/cmsis/LPC11xx.h"
#include "../system/peripheral/ssp.h"
#include "../util/adler32.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static const uint16_t START_DATA = 0xAB;

static volatile uint8_t temp_buf[400];

extern QueueHandle_t xSSP1RxQueue;
extern QueueHandle_t xSSP1TxQueue;

static void receive_and_check_data(size_t size, uint32_t dest_size,
		uint8_t* dest)
{
	volatile uint32_t i;

	for (i = 0; i < size; i += 2)
	{
		xQueueReceive(xSSP1RxQueue, &temp_buf[i], 0);
	}

	uint32_t sum = adler32(temp_buf, size - 4);

	uint32_t checksum = *(uint32_t*) ((uint8_t*) temp_buf + size - 4);

	if (checksum == sum)
	{
		memcpy(dest, temp_buf, dest_size);
	}
	else
	{
		uart_write_string("invalid checksum\r\n");
	}
}

static inline void send_data(size_t size, uint8_t* data)
{
	volatile uint32_t i;

	{
		xQueueSend(xSSP1TxQueue, &START_DATA, 0);
		for (i = 0; i < size; i += 2)
		{
			xQueueSend(xSSP1TxQueue, &data[i], 0);
		}
	}
}

void spi_transmit(void)
{
	volatile uint32_t data = 0;

	volatile size_t expect_size = 0;
	volatile uint8_t* data_ptr = 0;

	if (xQueueReceive(xSSP1RxQueue, &data, 0) == pdTRUE)
	{
		uint8_t func = (data >> 15) & 0x01;
		uint8_t addr = (data >> 8) & 0x7F;
		uint32_t size = ((uint32_t) (data & 0xFF) + 3) << 1; // at least 2 byte + checksum 4 bytes

		if (func == 0) // read
		{
			switch (addr)
			{
			case 1:
				expect_size = sizeof(fi_engine_state);
				data_ptr = (uint8_t*) &fi_engine_state;
				break;
			case 2:
				expect_size = sizeof(fi_switch_state);
				data_ptr = (uint8_t*) &fi_switch_state;
				break;
			}

			if (expect_size == size)
			{
				send_data(size, data_ptr);
			}
		}
		else // write
		{
			switch (addr)
			{
			case 1:
				expect_size = sizeof(fi_basic_setting);
				data_ptr = (uint8_t*) &fi_basic_setting;
				break;
			case 2:
				expect_size = sizeof(fi_starting_setting);
				data_ptr = (uint8_t*) &fi_starting_setting;
				break;
			case 3:
				expect_size = sizeof(fi_intake_temperature_correction);
				data_ptr = (uint8_t*) &fi_intake_temperature_correction;
				break;
			case 4:
				expect_size = sizeof(fi_oil_temperature_correction);
				data_ptr = (uint8_t*) &fi_oil_temperature_correction;
				break;
			case 5:
				expect_size = sizeof(fi_feedback_setting);
				data_ptr = (uint8_t*) &fi_feedback_setting;
				break;
			}

			if (expect_size == size)
			{
				receive_and_check_data(size, expect_size, data_ptr);
			}
		}
	}
}

