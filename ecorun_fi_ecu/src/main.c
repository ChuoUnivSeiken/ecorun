/*
 * main.c
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#include <string.h>
#include <stdlib.h>

#include "system/cmsis/LPC11xx.h"

#include "system/peripheral/uart.h"
#include "system/peripheral/ssp.h"
#include "system/peripheral/adc.h"
#include "system/peripheral/timer.h"
#include "system/peripheral/gpio.h"

#include "util/adler32.h"
#include "util/log2fix.h"

#include "carsystem/injection.h"
#include "carsystem/car_info.h"
#include "carsystem/system_io.h"
#include "carsystem/spi_transmit_data.h"

#include "FreeRTOS.h"
#include "task.h"

#include "fixmath/fix16.h"

static const uint32_t UART_CLK = 115200;
static const uint32_t FI_TASK_FREQ = 50;
static const uint32_t FI_COMM_TASK_FREQ = 5;
static const uint32_t FI_TIMER_FREQ = 50000;

/* # inject time
 *  step : 0.02ms
 *  min  : 0ms
 *  max  : 5.1ms
 */

extern volatile uint32_t adc_value[ADC_NUM];
extern volatile uint32_t adc_done_interrupt;

#define PRECISION 16

static uint32_t calc_ad_percentage(uint32_t ad_value)
{
	return (ad_value * 100) >> 10;
}

static uint32_t calc_ad_mv(uint32_t ad_value)
{
	return (ad_value * 3300) >> 10;
}

static uint32_t update_ad(uint32_t value, uint32_t num)
{
#define MAX_AD_SAMPLES 32
	static volatile uint32_t ad_samples[ADC_NUM][MAX_AD_SAMPLES];

	volatile uint32_t* ptr = ad_samples[value];

	uint32_t i;
	for (i = 0; i < num - 1; i++)
	{
		ptr[i] = ptr[i + 1];
	}
	ptr[num - 1] = adc_value[value];

	uint32_t sum = 0;
	for (i = 0; i < num; i++)
	{
		sum += ptr[i];
	}

	return sum / num;
}

static uint32_t compute_intake_temp(void)
{
	//        Vin------
	//                 |
	// Vcc |*---vvvvv--*--vv↑vv----| GND
	//　　　　　　　　r2=1k　　　　　　r1

	uint32_t vin = calc_ad_mv(update_ad(1, MAX_AD_SAMPLES)); // mV

	if (abs(3300 - vin) == 0)
	{
		return 0;
	}

	const uint32_t r2 = 1000; // Ω
	uint32_t r1 = vin * r2 / (3300 - vin); // Ω

	const uint32_t B = (3520 << PRECISION);
	const uint32_t T0 = 27 + 273;
	const uint32_t R0 = 1880;

	return (B
			/ (logfix(r1 << PRECISION, PRECISION)
					- logfix(R0 << PRECISION, PRECISION) + B / T0));
}

static uint32_t compute_oil_temp(void)
{
	//        Vin------
	//                 |
	// Vcc |*---vvvvv--*--vv↑vv----| GND
	//　　　　　　　　r2=1k　　　　　　r1

	uint32_t vin = calc_ad_mv(update_ad(0, MAX_AD_SAMPLES)); // mV

	if (abs(3300 - vin) == 0)
	{
		return 0;
	}

	const uint32_t r2 = 1000; // Ω
	uint32_t r1 = vin * r2 / (3300 - vin); // Ω

	const uint32_t B = (3520 << PRECISION);
	const uint32_t T0 = 27 + 273;
	const uint32_t R0 = 1880;

	return (B
			/ (logfix(r1 << PRECISION, PRECISION)
					- logfix(R0 << PRECISION, PRECISION) + B / T0));
}

static uint32_t compute_atmospheric_pressure()
{
	uint32_t vin = calc_ad_mv(update_ad(2, 5)); // mV

	static const uint32_t A = 466;
	static const uint32_t B = 0;

	uint32_t pressure = (((vin * A + 500) / 1000 + B));

	return pressure;
}

static uint32_t compute_throttle()
{
	return calc_ad_percentage(adc_value[3]);
}

static void update_io(void)
{
	fi_switch_state.starter_motor_on = gpio_get_value(3, 0);
	fi_switch_state.fuel_pump_on = gpio_get_value(3, 1);
	fi_switch_state.cdi_on = gpio_get_value(3, 2);

	set_starter_sw(fi_switch_state.starter_motor_on);
	set_fuel_sw(fi_switch_state.fuel_pump_on);
	set_cdi_sw(fi_switch_state.cdi_on);
}

static const uint32_t FUEL_CUT_THRESHOLD = 6000;

static void update_inject_time(void)
{
	adc_burst_read();

	while (!adc_done_interrupt)
	{
		// wait
	}

	fi_engine_state.th = compute_throttle();
	fi_engine_state.oil_temp = compute_oil_temp();
	fi_engine_state.intake_temp = compute_intake_temp();
	fi_engine_state.vacuum = compute_atmospheric_pressure();

	uint8_t inject_time = compute_base_inject_time();

	volatile uint8_t buf = inject_time;
	ssp_exchange(0, &buf, 1); // send current inject time and receive revolution.

	fi_engine_state.rev = buf;
	fi_engine_state.af = get_af();
	fi_engine_state.is_fuel_cut = fi_engine_state.rev > FUEL_CUT_THRESHOLD;

	fi_engine_state.checksum = adler32((uint8_t*) &fi_engine_state,
			sizeof(fi_engine_state) - sizeof(fi_engine_state.checksum));

	fi_feedback();
}

static const uint32_t stepper_rps = 4;

void init_fi_timer_clk(void)
{
	timer16_init(0, SystemCoreClock / FI_TIMER_FREQ);
	timer16_set_pwm(0, SystemCoreClock / FI_TIMER_FREQ);
	timer16_set_match(0, 2, SystemCoreClock / FI_TIMER_FREQ / 2);
	timer16_enable(0);

	uint32_t stepper_pulse_period = SystemCoreClock / (stepper_rps * 4);

	timer16_init(1, stepper_pulse_period);
	timer16_set_pwm(1, stepper_pulse_period);
	timer16_set_match(1, 2, stepper_pulse_period / 2);
	timer16_enable(1);
}

void vApplicationIdleHook(void)
{
	spi_transmit();
}

/*---------------------------------------------------------------------------*/

/* Priorities at which the tasks are created. */
#define FI_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define FI_TASK_FREQENCY_MS	( FI_TASK_FREQ / portTICK_RATE_MS )

static void fi_task(void* parameters)
{
	portTickType xNextWakeTime;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for (;;)
	{
		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time. */
		vTaskDelayUntil(&xNextWakeTime, FI_TASK_FREQENCY_MS);

		update_io();
		update_inject_time();
	}
}

/*---------------------------------------------------------------------------*/

/* Priorities at which the tasks are created. */
#define FI_COMM_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define FI_COMM_TASK_FREQENCY_MS	( FI_COMM_TASK_FREQ / portTICK_RATE_MS )

static void fi_comm_task(void* parameters)
{
	portTickType xNextWakeTime;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for (;;)
	{
		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time. */
		vTaskDelayUntil(&xNextWakeTime, FI_COMM_TASK_FREQENCY_MS);

		spi_transmit();
	}
}

/*---------------------------------------------------------------------------*/

int main(void)
{
	SystemCoreClockUpdate();

	gpio_init();

	init_io();

	set_fuel_sw(false);
	set_starter_sw(false);
	set_cdi_sw(false);

	fi_set_default();
	fi_engine_state.checksum = adler32((uint8_t*) &fi_engine_state,
			sizeof(fi_engine_state) - sizeof(fi_engine_state.checksum));

	ssp_init(1); // transmit with interface cpu
	ssp_init(0); // transmit with fi timer

	uart_init(UART_CLK);

	adc_init(ADC_CLK);

	init_fi_timer_clk();

	/* Create and start tasks */
	portBASE_TYPE result;

	if ((result = xTaskCreate(fi_task, "fi_task", configMINIMAL_STACK_SIZE,
	NULL,
	FI_TASK_PRIORITY, NULL)) != pdPASS)
	{
		//write_error("cannot create \"rxtx_eg_data\" task.", result);
	}

	//if ((result = xTaskCreate(fi_comm_task, "fi_comm_task", configMINIMAL_STACK_SIZE, NULL,
	//FI_COMM_TASK_PRIORITY, NULL)) != pdPASS)
	{
		//write_error("cannot create \"rxtx_eg_data\" task.", result);
	}

	/* Start the kernel.  From here on, only tasks and interrupts will run. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	 line will never be reached.  If the following line does execute, then there
	 was	insufficient FreeRTOS heap memory available for the idle and/or timer
	 tasks to be created.  See the memory management section on the FreeRTOS web
	 site, or the FreeRTOS tutorial books for more details. */
	for (;;)
		;

	return 0;
}

