/*-----------------------------------------------------------------------*/
/* Simple LED blinker program for MARY-MB  (C)ChaN, 2011                 */
/*-----------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include "cmsis/LPC11xx.h"

#include "peripheral/uart.h"
#include "peripheral/i2c.h"
#include "peripheral/ssp.h"

#include "type.h"
#include "command.h"
#include "peripheral/adc.h"
#include "peripheral/timer.h"
#include "gpio.h"
#include "integer.h"
#include "pin_config.h"

#include "carsystem/injection.h"
#include "carsystem/fi_settings.h"
#include "carsystem/car_info.h"

void timer32_0_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		adc_burst_read();

		volatile uint8_t inject_time = get_inject_time_from_map(eg_data.th, 0);

		volatile uint8_t buf = inject_time;
		ssp_exchange(0, &buf, 1);

		eg_data.rev = buf;

		eg_data.is_af_rich = (LPC_GPIO2->DATA & _BV(10)) ? 1 : 0;
	}
}

void adc_handler(uint8_t num, uint32_t value)
{
	switch (num)
	{
	case 1:
		eg_data.intake_temp = value;
		eg_data.checksum = adler32((uint8_t*) &eg_data,
				sizeof(eg_data) - sizeof(eg_data.checksum));
		break;
	case 2:
		eg_data.vacuum = value;
		eg_data.checksum = adler32((uint8_t*) &eg_data,
				sizeof(eg_data) - sizeof(eg_data.checksum));
		break;
	case 3:
		eg_data.th = value;
		eg_data.checksum = adler32((uint8_t*) &eg_data,
				sizeof(eg_data) - sizeof(eg_data.checksum));
		break;
	default:
		break;
	}
}

void set_starter_sw(bool on)
{
	if (on)
	{
		LPC_GPIO1->DATA |= _BV(11);
	}
	else
	{
		LPC_GPIO1->DATA &= ~_BV(11);
	}
}

void set_cdi_sw(bool on)
{
	if (on)
	{
		LPC_GPIO1->DATA |= _BV(4);
	}
	else
	{
		LPC_GPIO1->DATA &= ~_BV(4);
	}
}

void set_fuel_sw(bool on)
{
	if (on)
	{
		LPC_GPIO1->DATA |= _BV(3);
	}
	else
	{
		LPC_GPIO1->DATA &= ~_BV(3);
	}
}

void init_io_input_signals(void)
{
	LPC_IOCON->PIO2_10 = 0xd0;
	LPC_GPIO2->DIR &= ~_BV(10);
}

void init_io_output_signals(void)
{
	LPC_IOCON->SWDIO_PIO1_3 &= ~0x07; // fuel
	LPC_IOCON->SWDIO_PIO1_3 |= 0x01;

	LPC_IOCON->PIO1_4 &= ~0x07; // cdi
	LPC_IOCON->PIO1_4 |= 0x00;

	LPC_IOCON->PIO1_11 &= ~0x07; // starter
	LPC_IOCON->PIO1_11 |= 0x00;

	LPC_GPIO1->DATA &= ~_BV(3);
	LPC_GPIO1->DATA &= ~_BV(4);
	LPC_GPIO1->DATA &= ~_BV(11);

	LPC_GPIO1->DIR |= _BV(3);
	LPC_GPIO1->DIR |= _BV(4);
	LPC_GPIO1->DIR |= _BV(11);
}

void init_io(void)
{
	LPC_IOCON->R_PIO1_0 = 0x02; // adc 1 for throttle angle
	LPC_IOCON->R_PIO1_1 = 0x02; // adc 2 for throttle angle
	LPC_IOCON->R_PIO1_2 = 0x02; // adc 3 for throttle angle
	LPC_IOCON->SWCLK_PIO0_10 = 0xd3; // fi clock
	init_io_output_signals();
}

void init_fi_timer_clk(void)
{
	volatile uint32_t fi_timer_clk_freq = 256;
	timer16_init(0, 1000, SystemCoreClock / 1000 / fi_timer_clk_freq);
	timer16_set_pwm(0, SystemCoreClock / 1000 / fi_timer_clk_freq);
	timer16_set_match(0, 2, SystemCoreClock / 1000 / 2 / fi_timer_clk_freq);
	timer16_enable(0);
}

static volatile uint8_t ssp_fi_settings_buf[sizeof(fi_settings)];

void spi_transmit_blocking(void)
{
	volatile uint32_t data = 0;
	volatile uint8_t* eg_data_ptr = (uint8_t*) &eg_data;
	volatile uint32_t i;

	while (!(LPC_SSP1->SR & SSPSR_RNE))
		;

	__disable_irq();
	data = LPC_SSP1->DR;

	uint8_t func = (data >> 15) & 0x01;
	uint8_t addr = (data >> 8) & 0x7F;
	uint8_t size = data & 0xFF;

	if (func == 0) // read
	{
		if (addr == 0)
		{
			for (i = 0; i < size; i++)
			{
				while ((LPC_SSP1->SR & (SSPSR_TNF | SSPSR_BSY)) != SSPSR_TNF)
					;
				LPC_SSP1->DR = eg_data_ptr[i];
			}
		}
	}
	else // write
	{
		if (addr == 1)
		{
			for (i = 0; i < size; i++)
			{
				while (!(LPC_SSP1->SR & SSPSR_RNE))
					;
				ssp_fi_settings_buf[i] = LPC_SSP1->DR;
			}

			// validate data
			volatile uint32_t sum = adler32(ssp_fi_settings_buf, size - 4);

			volatile uint32_t checksum =
					*(uint32_t*) ((uint8_t*) ssp_fi_settings_buf
							+ sizeof(fi_settings.basic_inject_time_map));

			if (checksum == sum)
			{
				memcpy(&fi_settings, ssp_fi_settings_buf, sizeof(fi_settings));

				uart_writeln_string("fi_settings received.\r\n");
			}
		}
	}

	__enable_irq();
}

int main(void)
{
	SystemCoreClockUpdate();

	NVIC_SetPriority(ADC_IRQn, 1);
	NVIC_SetPriority(TIMER_32_0_IRQn, 2);

	fi_set_default();
	eg_data.checksum = adler32((uint8_t*) &eg_data,
			sizeof(eg_data) - sizeof(eg_data.checksum));

	uart_init(115200);

	init_io();

	ssp_init(1); // transmit with interface cpu
	ssp_init(0); // transmit with fi timer

	set_fuel_sw(true);

	adc_init(ADC_CLK);
	adc_add_event(adc_handler);

	timer32_init(0, SystemCoreClock / 100);
	timer_add_event_32_0(timer32_0_handler);
	timer32_enable(0);

	init_fi_timer_clk();

	uart_writeln_string("Hello, World!!\r\n");

	while (1)
	{
		spi_transmit_blocking();
	}

	return 0;
}

void HardFault_Handler(void)
{
	LPC_GPIO0->DATA = 0;
	LPC_GPIO1->DATA = 0;
}

