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

static volatile uint8_t ssp_fi_settings_buf[sizeof(fi_settings)];
static volatile uint8_t ssp_fi_feedback_settings_buf[sizeof(fi_feedback_settings)];

void spi_receive_fi_settings(uint32_t size)
{
	volatile uint32_t i;
	for (i = 0; i < size; i++)
	{
		while (!(LPC_SSP1->SR & SSPSR_RNE))
			;
		ssp_fi_settings_buf[i] = LPC_SSP1->DR;
	}

	volatile uint32_t sum = adler32(ssp_fi_settings_buf,
			sizeof(fi_settings.basic_inject_time_map));

	volatile uint32_t checksum = *(uint32_t*) ((uint8_t*) ssp_fi_settings_buf
			+ ((uint8_t*) &fi_settings.checksum
					- (uint8_t*) &fi_settings.basic_inject_time_map[0][0]));

	if (checksum == sum)
	{
		memcpy(&fi_settings, ssp_fi_settings_buf, sizeof(fi_settings));
	}
}

void spi_receive(uint32_t size, volatile uint8_t* temp_buf, uint32_t dest_size,
		volatile uint8_t* dest)
{
	volatile uint32_t i;

	__disable_irq();
	{
		for (i = 0; i < size; i++)
		{
			while (!(LPC_SSP1->SR & SSPSR_RNE))
				;
			temp_buf[i] = LPC_SSP1->DR;
		}
	}
	__enable_irq();

	volatile uint32_t sum = adler32(temp_buf, size - 4);

	volatile uint32_t checksum = *(uint32_t*) ((uint8_t*) temp_buf + size - 4);

	if (checksum == sum)
	{
		memcpy(dest, temp_buf, dest_size);
	}
}

void spi_send_data(volatile uint8_t* data, uint32_t size)
{
	volatile uint32_t i;

	__disable_irq();
	{
		for (i = 0; i < size; i++)
		{
			while ((LPC_SSP1->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
				;
			LPC_SSP1->DR = data[i];
		}
	}
	__enable_irq();
}

void spi_transmit_blocking(void)
{
	volatile uint32_t data = 0;

	while (!(LPC_SSP1->SR & SSPSR_RNE))
		;
	data = LPC_SSP1->DR;

	uint8_t func = (data >> 15) & 0x01;
	uint8_t addr = (data >> 8) & 0x7F;
	uint32_t size = (uint32_t) (data & 0xFF) + 1;

	if (func == 0) // read
	{
		if (addr == 0)
		{
			spi_send_data((uint8_t*) &eg_data, size);
		}
	}
	else // write
	{
		if (addr == 1)
		{
			//spi_receive_fi_settings(size);
			spi_receive(size, ssp_fi_settings_buf, sizeof(fi_settings),
					(uint8_t*) &fi_settings);
		}
		else if (addr == 2)
		{
			spi_receive(size, ssp_fi_feedback_settings_buf,
					sizeof(fi_feedback_settings),
					(uint8_t*) &fi_feedback_settings);
		}
	}
}

