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

static volatile uint8_t ssp_fi_settings_buf[256 + 4];

static void receive_and_check_data(size_t size, uint8_t* temp_buf,
		uint32_t dest_size, uint8_t* dest)
{
	ssp_receive(1, (uint16_t*) temp_buf, size >> 1);

	volatile uint32_t sum = adler32(temp_buf, size - 4);

	volatile uint32_t checksum = *(uint32_t*) ((uint8_t*) temp_buf + size - 4);

	if (checksum == sum)
	{
		memcpy(dest, temp_buf, dest_size);
	}
	else
	{
		uart_write_string("invalid checksum\r\n");
	}
}

void spi_transmit_blocking(void)
{
	volatile uint32_t data = 0;

	while (!(LPC_SSP1->SR & SSPSR_RNE))
		;

	__disable_irq();

	data = LPC_SSP1->DR;

	if (data == 0xFFFF)
	{
		return;
	}

	uint8_t func = (data >> 15) & 0x01;
	uint8_t addr = (data >> 8) & 0x7F;
	uint32_t size = ((uint32_t) (data & 0xFF) + 3) << 1; // at least 2 byte + checksum 4 bytes

	if (func == 0) // read
	{
		if (addr == 0)
		{
			ssp_send_uint16(1, (uint16_t*) &eg_data, size >> 1);
		}
	}
	else // write
	{
		if (addr == 1)
		{
			//spi_receive_fi_settings(size);
			receive_and_check_data(size, ssp_fi_settings_buf,
					sizeof(fi_settings), (uint8_t*) &fi_settings);
		}
		else if (addr == 2)
		{
			receive_and_check_data(size, ssp_fi_settings_buf,
					sizeof(fi_feedback_settings),
					(uint8_t*) &fi_feedback_settings);
		}
	}

	__enable_irq();
}

