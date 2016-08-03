/*
 * main.c
 *
 *  Created on: 2015/03/12
 *      Author: Yoshio
 */

#include "system/cmsis/LPC13Uxx.h"
#include "system/peripheral/usart.h"
#include "system/peripheral/timer.h"
#include "system/peripheral/adc.h"
#include "system/peripheral/ssp.h"
#include "system/peripheral/eeprom.h"
#include "system/systimer.h"
#include "util/usart_util.h"
#include "core/command.h"
#include "core/base64.h"
#include "core/adler32.h"
#include "integer.h"
#include "carsystem/car_info.h"
#include "carsystem/injection.h"
#include "carsystem/fi_settings.h"
#include "carsystem/accessible_data.h"
#include <string.h>
#include "countdown_timer.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define USE_BLUETOOTH 0

#define SSP_MAX_DATA_SIZE (512 + 4)

volatile uint8_t ssp_buf[SSP_MAX_DATA_SIZE];

static uint16_t spi_encode_header(uint32_t func, uint32_t addr, size_t size)
{
	return (((func & 0x01) << 15) | ((addr & 0x7F) << 8)
			| (((size - 3) >> 1) & 0xFF));
}

static void receive_and_send_eneine_data(void)
{
	// synchronizing
	{
		volatile uint32_t i = 0;
		for (i = 0; i < (SSP_MAX_DATA_SIZE >> 1); i++)
		{
			uint16_t data = 0xFFFF;
			ssp_send_uint16(0, &data, 1);
		}
	}

	// receive engine data
	{
		const size_t size = sizeof(eg_data);
		const uint16_t data = spi_encode_header(0x00, 0x00, size);

		{
			ssp_send_uint16(0, &data, 1);
			ssp_receive_uint16(0, (uint16_t*) ssp_buf, size >> 1);
		}

		const uint32_t checksum = *(uint32_t*) ((uint8_t*) ssp_buf + size - 4);

		const uint32_t sum = adler32((const_buffer) ssp_buf, size - 4);

		if (sum == checksum)
		{
			memcpy(&eg_data, ssp_buf, sizeof(eg_data));
		}
		else
		{
			usart_writeln_string("msg <checksum of data from is invalid.>");
		}
	}

	// send injection map
	{
		volatile uint8_t* fi_settings_ptr = (uint8_t*) &fi_settings;

		fi_settings.checksum = adler32((const_buffer) &fi_settings,
				sizeof(fi_settings) - sizeof(fi_settings.checksum));

		const uint32_t size = sizeof(fi_settings);
		const uint16_t data = spi_encode_header(0x01, 0x01, size);

		{
			ssp_send_uint16(0, &data, 1);
			ssp_send_uint16(0, (uint16_t*) fi_settings_ptr, size >> 1);
		}
	}
}

typedef struct
{
	uint8_t switches[5];
	uint8_t leds[4];
} io_info;

static io_info io_info_inst;

static void monitor_switch(void)
{
	// send switch states to ecu pin
	io_info_inst.switches[0] = (LPC_GPIO->PIN[1] & _BV(4)) == 0;
	io_info_inst.switches[1] = (LPC_GPIO->PIN[1] & _BV(27)) == 0;
	io_info_inst.switches[2] = (LPC_GPIO->PIN[1] & _BV(26)) == 0;

	if (LPC_GPIO->PIN[1] & _BV(4))
	{
		LPC_GPIO->CLR[1] |= _BV(11);
	}
	else
	{
		LPC_GPIO->SET[1] |= _BV(11);
	}

	if (LPC_GPIO->PIN[1] & _BV(27))
	{
		LPC_GPIO->CLR[1] |= _BV(29);
	}
	else
	{
		LPC_GPIO->SET[1] |= _BV(29);
	}

	if (LPC_GPIO->PIN[1] & _BV(26))
	{
		LPC_GPIO->CLR[0] |= _BV(22);
	}
	else
	{
		LPC_GPIO->SET[0] |= _BV(22);
	}
}

static void monitor_throttle(void)
{
	volatile uint32_t angle = ((1800 * eg_data.th) >> 10);
	volatile uint32_t match = ((SystemCoreClock / 1000000) * (angle + 500));
	timer32_set_match(1, 1, match);
}

typedef struct
{
	uint8_t major_version;
	uint8_t minor_version;
	uint8_t injection_map_th_points;
	uint8_t injection_map_rev_points;
} device_info;

static device_info dev_info;

void command_error_func(const char* id, command_func func)
{
	usart_write_string("msg <can't register command : ");
	usart_write_string(id);
	usart_writeln_string(">");
}

static const named_data register_data_table[] =
{
{ "device_info", (void*) &dev_info, sizeof(dev_info), true },
{ "engine_data", (void*) &eg_data, sizeof(eg_data), true },
{ "basic_inject_time_map", (void*) &fi_settings.basic_inject_time_map[0][0],
		sizeof(fi_settings.basic_inject_time_map), false },
{ "io_info", (void*) &io_info_inst, sizeof(io_info_inst), true },
{ "car_data", (void*) &cr_data, sizeof(cr_data), true } };

void command_get(command_data* data)
{
	const_string id = data->args[0].arg_value;

	find_and_put_data(id);
}

static inline void write_message(const_string msg)
{
	usart_write_string("msg <");
	usart_write_string(msg);
	usart_writeln_string(">");
}

static inline void write_error(const_string msg, portBASE_TYPE code)
{
	usart_write_string("msg <");
	usart_write_string(msg);
	usart_write_string(", code ");
	usart_write_int32((int) code);
	usart_writeln_string(">");
}

void command_put(command_data* data)
{
	static volatile uint8_t buffer[512];

	const uint8_t* id = data->args[0].arg_value;
	const uint32_t encoded_size = str_to_uint32(data->args[1].arg_value);
	const_string encoded_data = data->args[2].arg_value;
	const uint32_t sum = str_to_uint32(data->args[3].arg_value);

	named_data registered_data;
	if (!find_data(id, &registered_data))
	{
		write_message("cannot find the data.");
	}

	if (registered_data.is_read_only)
	{
		write_message("the data is readonly.");
		return;
	}
	uint32_t decoded_size = decode_base64(encoded_data, NULL);
	uint32_t size = registered_data.data_size;
	if (size != decoded_size)
	{
		write_message("don't match the data size.");
		return;
	}
	if (size > sizeof(buffer))
	{
		write_message("the data size is too long.");
		return;
	}
	decode_base64_s(encoded_data, (uint8_t*) buffer, 512);

	const uint32_t check_sum = adler32(buffer, decoded_size);

	if (check_sum != sum)
	{
		write_message("don't match the check sum.");
		return;
	}

	memcpy((uint8_t*) registered_data.data_ptr, buffer, size);
}

void command_exec(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	if (strcmp(id, "save-settings") == 0)
	{
		eeprom_write((uint8_t*) 0, (buffer) fi_settings.basic_inject_time_map,
				sizeof(fi_settings.basic_inject_time_map));

		write_message("settings saved.");
	}
	else if (strcmp(id, "load-settings") == 0)
	{
		eeprom_read((uint8_t*) 0, (buffer) fi_settings.basic_inject_time_map,
				sizeof(fi_settings.basic_inject_time_map));

		write_message("settings loaded.");
	}
}

void adc_handler(uint8_t num, uint32_t value)
{
	switch (num)
	{
	case 1:
		break;
	case 2:
		break;
	case 4:
		break;
	case 5:
		break;
	}
}

void init_io_adc(void)
{
	/* P0.11 = ADC0 */
	LPC_IOCON->TDI_PIO0_11 &= ~0x9F;
	LPC_IOCON->TDI_PIO0_11 |= 0x02;
	/* P0.12 = ADC1 */
	LPC_IOCON->TMS_PIO0_12 &= ~0x9F;
	LPC_IOCON->TMS_PIO0_12 |= 0x02;
	/* P0.13 = ADC2 */
	LPC_IOCON->TDO_PIO0_13 &= ~0x9F;
	LPC_IOCON->TDO_PIO0_13 |= 0x02;
	/* P0.14 = ADC3 */
	LPC_IOCON->TRST_PIO0_14 &= ~0x9F;
	LPC_IOCON->TRST_PIO0_14 |= 0x02;
	/* P0.15 = ADC4 ... this is also SWDIO so be careful with this pin! */
	LPC_IOCON->SWDIO_PIO0_15 &= ~0x9F;
	LPC_IOCON->SWDIO_PIO0_15 |= 0x02;
	/* P0.16 = ADC5 */
	LPC_IOCON->PIO0_16 &= ~0x9F;
	LPC_IOCON->PIO0_16 |= 0x01;
	/* P0.22 = ADC6 */
	LPC_IOCON->PIO0_22 &= ~0x9F;
	LPC_IOCON->PIO0_22 |= 0x01;
	/* P0.23 = ADC7 */
	LPC_IOCON->PIO0_23 &= ~0x9F;
	LPC_IOCON->PIO0_23 |= 0x01;
}

void init_io_ssp0(void)
{
	/* Set 0.8 to SSP MISO0 */
	LPC_IOCON->PIO0_8 &= ~0x07;
	LPC_IOCON->PIO0_8 |= 0x01;

	/* Set 0.9 to SSP MOSI0 */
	LPC_IOCON->PIO0_9 &= ~0x07;
	LPC_IOCON->PIO0_9 |= 0x01;

	/* Set 0.10 to SSP SCK0 (may be required for SWD!) */
	LPC_IOCON->SWCLK_PIO0_10 = 0x02;

	LPC_IOCON->PIO0_2 &= ~0x07;
	LPC_IOCON->PIO0_2 |= 0x01;
}

void init_io_ssp1(void)
{
	/* Set P1.22 to SSP MOSI1 */
	LPC_IOCON->PIO1_22 &= ~0x07;
	LPC_IOCON->PIO1_22 |= 0x02;

	/* Set P1.21 to SSP MISO1 */
	LPC_IOCON->PIO1_21 &= ~0x07;
	LPC_IOCON->PIO1_21 |= 0x02;

	/* Set 1.20 to SSP SCK1 */
	LPC_IOCON->PIO1_20 &= ~0x07;
	LPC_IOCON->PIO1_20 |= 0x02;

	/* Set 1.23 to SSP SSEL1 */
	LPC_IOCON->PIO1_23 &= ~0x07;
	LPC_IOCON->PIO1_23 |= 0x00;
	LPC_GPIO->DIR[1] |= _BV(23);
	LPC_GPIO->SET[1] |= _BV(23);
}

void init_io_usart(void)
{
#if USE_BLUETOOTH
	/* Set 1.13 UART RXD */
	LPC_IOCON->PIO1_13 &= ~0x07;
	LPC_IOCON->PIO1_13 |= 0x03;

	/* Set 1.14 UART TXD */
	LPC_IOCON->PIO1_14 &= ~0x07;
	LPC_IOCON->PIO1_14 |= 0x03;
#else
	/* Set 0.18 UART RXD */
	LPC_IOCON->PIO0_18 &= ~0x07;
	LPC_IOCON->PIO0_18 |= 0x01;

	/* Set 0.19 UART TXD */
	LPC_IOCON->PIO0_19 &= ~0x07;
	LPC_IOCON->PIO0_19 |= 0x01;
#endif
}

void init_io_signals(void)
{
	LPC_IOCON->PIO1_4 = 0x10; // sw0
	LPC_IOCON->PIO1_27 = 0x10; // sw1
	LPC_IOCON->PIO1_26 = 0x10; // sw2
	LPC_IOCON->PIO1_10 = 0x10; // sw3
	LPC_IOCON->PIO0_20 = 0x10; // sw4

	LPC_GPIO->DIR[1] &= ~_BV(4);
	LPC_GPIO->DIR[1] &= ~_BV(27);
	LPC_GPIO->DIR[1] &= ~_BV(26);
	LPC_GPIO->DIR[1] &= ~_BV(10);
	LPC_GPIO->DIR[0] &= ~_BV(20);

	LPC_IOCON->PIO1_0 = 0x10; // led0
	LPC_IOCON->PIO1_25 = 0x10; // led1
	LPC_IOCON->PIO1_19 = 0x10; // led2
	LPC_IOCON->PIO1_7 = 0x10; // led3

	LPC_GPIO->DIR[1] |= _BV(0);
	LPC_GPIO->DIR[1] |= _BV(25);
	LPC_GPIO->DIR[1] |= _BV(19);
	LPC_GPIO->DIR[1] |= _BV(7);

	LPC_GPIO->CLR[1] |= _BV(0);
	LPC_GPIO->CLR[1] |= _BV(25);
	LPC_GPIO->CLR[1] |= _BV(19);
	LPC_GPIO->CLR[1] |= _BV(7);

	LPC_IOCON->PIO1_11 = 0x10; // fuel output
	LPC_IOCON->PIO1_29 = 0x10; // cdi output
	LPC_IOCON->PIO0_22 = 0x10; // starter output

	LPC_GPIO->DIR[1] |= _BV(11);
	LPC_GPIO->DIR[1] |= _BV(29);
	LPC_GPIO->DIR[0] |= _BV(22);

	LPC_GPIO->CLR[1] |= _BV(11);
	LPC_GPIO->CLR[1] |= _BV(29);
	LPC_GPIO->CLR[0] |= _BV(22);

	LPC_IOCON->TRST_PIO0_14 = 0x13; // servo
}

void init_io(void)
{
	//init_io_adc();
	init_io_ssp0();
	init_io_usart();
	init_io_signals();
}

void init_cli(void)
{
	const uint32_t USART_BAUDRATE = 115200;
	usart_init(USART_BAUDRATE);
	usart_handler_init();

	initialize_command_system(command_error_func);

	uint32_t registered_data_count = sizeof(register_data_table)
			/ sizeof(register_data_table[0]);
	register_data(register_data_table, registered_data_count);
	register_command("get", command_get);
	register_command("put", command_put);
	register_command("exec", command_exec);
}

/* Priorities at which the tasks are created. */
#define EXECUTE_COMMAND_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define EXECUTE_COMMAND_TASK_FREQENCY_MS	( 20 / portTICK_RATE_MS )

static void execute_command_task(void* parameters)
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
		vTaskDelayUntil(&xNextWakeTime, EXECUTE_COMMAND_TASK_FREQENCY_MS);

		execute_one_command();
	}
}

/* Priorities at which the tasks are created. */
#define MONITOR_SWITCH_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define MONITOR_SWITCH_TASK_FREQENCY_MS	( 20 / portTICK_RATE_MS )

static void monitor_switch_task(void* parameters)
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
		vTaskDelayUntil(&xNextWakeTime, MONITOR_SWITCH_TASK_FREQENCY_MS);

		monitor_switch();
	}
}

/* Priorities at which the tasks are created. */
#define MONITOR_THROTTLE_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define MONITOR_THROTTLE_TASK_FREQENCY_MS	( 20 / portTICK_RATE_MS )

static void monitor_throttle_task(void* parameters)
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
		vTaskDelayUntil(&xNextWakeTime, MONITOR_THROTTLE_TASK_FREQENCY_MS);

		monitor_throttle();
	}
}

/* Priorities at which the tasks are created. */
#define RXTX_ENGINE_DATA_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define RXTX_ENGINE_DATA_TASK_FREQENCY_MS	( 20 / portTICK_RATE_MS )

static void rxtx_engine_data_task(void* parameters)
{
	portTickType xNextWakeTime;

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	ssp_init(0);

	for (;;)
	{
		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time. */
		vTaskDelayUntil(&xNextWakeTime, RXTX_ENGINE_DATA_TASK_FREQENCY_MS);

		receive_and_send_eneine_data();
	}
}

static void start_scheduler(void)
{
	/* Create and start tasks */
	portBASE_TYPE result;

	if ((result = xTaskCreate(execute_command_task, "execute_command",
			configMINIMAL_STACK_SIZE, NULL, EXECUTE_COMMAND_TASK_PRIORITY, NULL))
			!= pdPASS)
	{
		write_error("cannot create \"execute_command\" task.", result);
	}
	if ((result = xTaskCreate(monitor_switch_task, "monitor_switch",
			configMINIMAL_STACK_SIZE, NULL, MONITOR_SWITCH_TASK_PRIORITY, NULL))
			!= pdPASS)
	{
		write_error("cannot create \"monitor_switch\" task.", result);
	}

	if ((result = xTaskCreate(rxtx_engine_data_task, "rxtx_eg_data",
			configMINIMAL_STACK_SIZE, NULL, RXTX_ENGINE_DATA_TASK_PRIORITY,
			NULL)) != pdPASS)
	{
		write_error("cannot create \"rxtx_eg_data\" task.", result);
	}

	if ((result = xTaskCreate(monitor_throttle_task, "monitor_throttle",
			configMINIMAL_STACK_SIZE, NULL, MONITOR_THROTTLE_TASK_PRIORITY,
			NULL)) != pdPASS)
	{
		write_error("cannot create \"monitor_throttle\" task.", result);
	}

	/* Start the tasks running. */
	vTaskStartScheduler();
}

int main(void)
{
	SystemCoreClockUpdate();

	init_io();

	NVIC_SetPriority(USART_IRQn, 1);
	NVIC_SetPriority(CT32B0_IRQn, 2);

	eeprom_read((uint8_t*) 0, (buffer) fi_settings.basic_inject_time_map,
			sizeof(fi_settings.basic_inject_time_map));

	//systimer_init();

	init_cli();

	timer32_init(1, SystemCoreClock / 1000000 * 15000);
	timer32_set_pwm(1, SystemCoreClock / 1000000 * 15000);
	//timer32_set_match(1, 1, SystemCoreClock * 7 / 10000);
	timer32_enable(1);

	//countdown_timer_init();

	dev_info.major_version = 1;
	dev_info.minor_version = 0;
	dev_info.injection_map_rev_points = 13;
	dev_info.injection_map_th_points = 11;

	start_scheduler();

	/* If all is well we will never reach here as the scheduler will now be
	 running.  If we do reach here then it is likely that there was insufficient
	 heap available for the idle task to be created. */
	for (;;)
		;

	return 0;
}
