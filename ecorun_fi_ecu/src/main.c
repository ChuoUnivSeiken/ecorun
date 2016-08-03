/*
 * main.c
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#include <string.h>
#include "system/cmsis/LPC11xx.h"

#include "system/peripheral/uart.h"
#include "system/peripheral/ssp.h"
#include "system/peripheral/adc.h"
#include "system/peripheral/timer.h"
#include "system/peripheral/gpio.h"

#include "util/adler32.h"

#include "carsystem/injection.h"
#include "carsystem/car_info.h"
#include "carsystem/system_io.h"
#include "carsystem/spi_transmit_data.h"

#define UART_CLK 115200
#define FI_TASK_FREQ 100
#define FI_TIMER_FREQ 50000

void timer32_0_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		fi_switch_info.starter_motor_on = gpio_get_value(3, 0);
		fi_switch_info.fuel_pump_on = gpio_get_value(3, 1);
		fi_switch_info.cdi_on = gpio_get_value(3, 2);

		adc_burst_read();

		volatile uint8_t inject_time = get_inject_time_from_map();

		volatile uint8_t buf = inject_time;
		ssp_exchange(0, &buf, 1); // send current inject time and receive revolution.

		eg_data.rev = buf;
		eg_data.is_af_rich = get_af();

		fi_feedback();

		set_starter_sw(fi_switch_info.starter_motor_on);
		set_fuel_sw(fi_switch_info.fuel_pump_on);
		set_cdi_sw(fi_switch_info.cdi_on);
	}
}

void adc_handler(uint8_t num, uint32_t value)
{
	switch (num)
	{
	case 0: // oil temperature sensor
		eg_data.oil_temp = value;
		break;
	case 1: // intake temperature sensor
		eg_data.intake_temp = value;
		break;
	case 2: // vacuum sensor
		eg_data.vacuum = value;
		break;
	case 3: // throttle angle sensor
		eg_data.th = value;
		break;
	case 4: // o2 sensor
		//eg_data.o2 = value;
		break;
	case 5: // input voltage sensor
		//eg_data.input_voltage = value;
		break;
	case 6: // unused
	case 7: // unused
	default:
		break;
	}
	eg_data.checksum = adler32((uint8_t*) &eg_data,
			sizeof(eg_data) - sizeof(eg_data.checksum));
}

void init_fi_timer_clk(void)
{
	timer16_init(0, SystemCoreClock / FI_TIMER_FREQ);
	timer16_set_pwm(0, SystemCoreClock / FI_TIMER_FREQ);
	timer16_set_match(0, 2, SystemCoreClock / FI_TIMER_FREQ / 2);
	timer16_enable(0);
}

int main(void)
{
	SystemCoreClockUpdate();

	gpio_init();

	init_io();

	NVIC_SetPriority(ADC_IRQn, 1);
	NVIC_SetPriority(TIMER_32_0_IRQn, 2);

	fi_set_default();
	eg_data.checksum = adler32((uint8_t*) &eg_data,
			sizeof(eg_data) - sizeof(eg_data.checksum));

	uart_init(UART_CLK);

	ssp_init(1); // transmit with interface cpu
	ssp_init(0); // transmit with fi timer

	set_fuel_sw(true);

	adc_init(ADC_CLK);
	adc_add_event(adc_handler);

	timer32_init(0, SystemCoreClock / FI_TASK_FREQ);
	timer32_add_event(0, timer32_0_handler);
	timer32_enable(0);

	init_fi_timer_clk();

	while (1)
	{
		spi_transmit_blocking();
	}

	return 0;
}

