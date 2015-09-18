#include <string.h>
#include <stdio.h>
#include "system/cmsis/LPC11xx.h"

#include "system/peripheral/uart.h"
#include "system/peripheral/i2c.h"
#include "system/peripheral/ssp.h"
#include "system/peripheral/adc.h"
#include "system/peripheral/timer.h"
#include "system/peripheral/gpio.h"

#include "carsystem/injection.h"
#include "carsystem/fi_settings.h"
#include "carsystem/car_info.h"
#include "carsystem/system_io.h"
#include "carsystem/spi_transmit_data.h"

void timer32_0_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		bool starter_on = (LPC_GPIO3->DATA & _BV(0)) != 0;

		adc_burst_read();

		volatile uint8_t inject_time = get_inject_time_from_map(eg_data.th,
				eg_data.rev);

		if (starter_on)
		{
			inject_time = 100;
		}
#if 0
		char numbuff[20];
		uint32_to_str(inject_time, numbuff);
		uart_write_string(numbuff);
		uart_writeln_string("\r\n");
#endif

		volatile uint8_t buf = inject_time;
		ssp_exchange(0, &buf, 1); // send current inject time and receive revolution.

		eg_data.rev = buf;
		eg_data.is_af_rich = get_af();

		fi_feedback();

		set_starter_sw(starter_on);
		set_fuel_sw((LPC_GPIO3->DATA & _BV(1)) != 0);
		set_cdi_sw((LPC_GPIO3->DATA & _BV(2)) != 0);
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

void init_fi_timer_clk(void)
{
	volatile uint32_t fi_timer_clk_freq = 50000;
	timer16_init(0, 1, SystemCoreClock / fi_timer_clk_freq);
	timer16_set_pwm(0, SystemCoreClock / fi_timer_clk_freq);
	timer16_set_match(0, 2, SystemCoreClock / 2 / fi_timer_clk_freq);
	timer16_enable(0);
}

int main(void)
{
	SystemCoreClockUpdate();

	init_io();

	NVIC_SetPriority(ADC_IRQn, 1);
	NVIC_SetPriority(TIMER_32_0_IRQn, 2);

	fi_set_default();
	eg_data.checksum = adler32((uint8_t*) &eg_data,
			sizeof(eg_data) - sizeof(eg_data.checksum));

	uart_init(115200);

	ssp_init(1); // transmit with interface cpu
	ssp_init(0); // transmit with fi timer

	set_fuel_sw(true);

	adc_init(ADC_CLK);
	adc_add_event(adc_handler);

	timer32_init(0, SystemCoreClock / 100);
	timer_add_event_32_0(timer32_0_handler);
	timer32_enable(0);

	init_fi_timer_clk();

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

