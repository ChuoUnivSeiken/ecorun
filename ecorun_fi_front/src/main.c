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
#include "carsystem/accessible_data.h"
#include <string.h>
#include <stdlib.h>
#include "json/jsmn.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define USE_BLUETOOTH 1

xSemaphoreHandle usart_mutex = NULL;

typedef struct eeprom_entry_t
{
	const char* name;
	uint8_t* data;
	size_t size;
	uint32_t offset;
} eeprom_entry;

#define EEPROM_ENTRY(data) { #data, (uint8_t*) &data, sizeof(data), -1 }

static eeprom_entry eeprom_entries[] =
{
EEPROM_ENTRY(fi_basic_setting),
EEPROM_ENTRY(fi_feedback_setting),
EEPROM_ENTRY(fi_starting_setting),
EEPROM_ENTRY(fi_intake_temperature_correction),
EEPROM_ENTRY(fi_oil_temperature_correction), };

#define NUM_EEPRON_ENTRIES (sizeof(eeprom_entries) / sizeof(eeprom_entries[0]))

/*---------------------------------------------------------------------------*/

static inline void write_message(const_string msg)
{
	if (usart_mutex)
	{
		xSemaphoreTake(usart_mutex, portMAX_DELAY);
	}
	usart_write_string("msg <");
	usart_write_string(msg);
	usart_writeln_string(">");
	if (usart_mutex)
	{
		xSemaphoreGive(usart_mutex);
	}
}

static inline void write_error(const_string msg, portBASE_TYPE code)
{
	if (usart_mutex)
	{
		xSemaphoreTake(usart_mutex, portMAX_DELAY);
	}
	usart_write_string("msg <");
	usart_write_string(msg);
	usart_write_string(", code ");
	usart_write_int32((int) code);
	usart_writeln_string(">");
	if (usart_mutex)
	{
		xSemaphoreGive(usart_mutex);
	}
}

/*---------------------------------------------------------------------------*/

#define SSP_MAX_DATA_SIZE (512 + 4)

static uint16_t spi_encode_header(uint32_t func, uint32_t addr, size_t size)
{
	return (((func & 0x01) << 15) | ((addr & 0x7F) << 8)
			| (((size >> 1) - 3) & 0xFF));
}

static const uint16_t START_DATA = 0xAB;

static void send_data_to_ecu(const uint8_t* data, size_t size, size_t address)
{
	const uint16_t header = spi_encode_header(0x01, 0x01, size);

	portENTER_CRITICAL();
	{
		ssp_send_uint16(0, &header, 1);
		ssp_send_uint16(0, (const uint16_t*) data, size >> 1);
	}
	portEXIT_CRITICAL();
}

static void receive_data_from_ecu(uint8_t* data, size_t size, size_t address)
{
	const uint16_t header = spi_encode_header(0x00, 0x01, size);

	{
		ssp_send_uint16(0, &header, 1);

		volatile uint16_t res = 0;

		volatile int32_t retry = 10;
		do
		{
			vTaskDelay(2);
			ssp_receive_uint16(0, (uint16_t*) &res, 1);
		} while (res != START_DATA && retry-- > 0); // Wait for card goes ready or timeout

		ssp_receive_uint16(0, (uint16_t*) data, size >> 1);
	}
}

static void receive_and_send_eneine_data(void)
{
	static volatile uint8_t ssp_buf[SSP_MAX_DATA_SIZE];

	// receive engine data
	{
		receive_data_from_ecu((uint8_t*) ssp_buf, sizeof(fi_engine_state),
				0x01);

		const uint32_t checksum =
				*(uint32_t*) ((uint8_t*) ssp_buf
						+ (size_t) &fi_engine_state.checksum
						- (size_t) &fi_engine_state);

		const uint32_t sum = adler32((uint8_t*) ssp_buf,
				(size_t) &fi_engine_state.checksum - (size_t) &fi_engine_state);

		if (sum == checksum)
		{
			memcpy(&fi_engine_state, ssp_buf, sizeof(fi_engine_state));
		}
		else
		{
			write_message("checksum of data from ecu is invalid.");
		}
	}

	// send settings
	{
		{
			fi_basic_setting.checksum = adler32(
					(const_buffer) &fi_basic_setting,
					sizeof(fi_basic_setting)
							- sizeof(fi_basic_setting.checksum));

			send_data_to_ecu((uint8_t*) &fi_basic_setting,
					sizeof(fi_basic_setting), 0x01);
		}

		{
			fi_starting_setting.checksum = adler32(
					(const_buffer) &fi_starting_setting,
					sizeof(fi_starting_setting)
							- sizeof(fi_starting_setting.checksum));

			send_data_to_ecu((uint8_t*) &fi_starting_setting,
					sizeof(fi_starting_setting), 0x02);
		}

		{
			fi_intake_temperature_correction.checksum =
					adler32((const_buffer) &fi_intake_temperature_correction,
							sizeof(fi_intake_temperature_correction)
									- sizeof(fi_intake_temperature_correction.checksum));

			send_data_to_ecu((uint8_t*) &fi_intake_temperature_correction,
					sizeof(fi_intake_temperature_correction), 0x03);
		}

		{
			fi_oil_temperature_correction.checksum = adler32(
					(const_buffer) &fi_oil_temperature_correction,
					sizeof(fi_oil_temperature_correction)
							- sizeof(fi_oil_temperature_correction.checksum));

			send_data_to_ecu((uint8_t*) &fi_oil_temperature_correction,
					sizeof(fi_oil_temperature_correction), 0x04);
		}

		{
			fi_feedback_setting.checksum = adler32(
					(const_buffer) &fi_feedback_setting,
					sizeof(fi_feedback_setting)
							- sizeof(fi_feedback_setting.checksum));

			send_data_to_ecu((uint8_t*) &fi_feedback_setting,
					sizeof(fi_feedback_setting), 0x05);
		}
	}
}

/*---------------------------------------------------------------------------*/

typedef struct
{
	uint8_t switches[5];
	uint8_t leds[4];
} io_info_data;

static io_info_data io_info;

typedef struct
{
	uint8_t major_version;
	uint8_t minor_version;
	uint8_t injection_map_th_points;
	uint8_t injection_map_rev_points;
} device_info_data;

static device_info_data device_info;

#define DATA_TABLE_ENTRY(name, data, is_readonly) { name, (void*) &data, sizeof(data), is_readonly }

static const accessible_data_entry register_data_table[] =
{
DATA_TABLE_ENTRY("device_info", device_info, true),
DATA_TABLE_ENTRY("fi_engine_state", fi_engine_state, true),
DATA_TABLE_ENTRY("fi_basic_setting", fi_basic_setting, false),
DATA_TABLE_ENTRY("fi_feedback_setting", fi_feedback_setting, false),
DATA_TABLE_ENTRY("fi_starting_setting", fi_starting_setting, false),
DATA_TABLE_ENTRY("fi_intake_temperature_correction",
		fi_intake_temperature_correction, false),
DATA_TABLE_ENTRY("fi_oil_temperature_correction",
		fi_oil_temperature_correction, false),
DATA_TABLE_ENTRY("io_info", io_info, true),
DATA_TABLE_ENTRY("car_data", cr_data, true), };

void command_error_func(const char* id, command_func func)
{
	usart_write_string("msg <can't register command : ");
	usart_write_string(id);
	usart_writeln_string(">");
}

typedef struct
{
	const char* method;
	uint32_t unique_id;

	const char* id;
	uint32_t size;
	const char* data;
	uint32_t checksum;
} request_data;

void request_get(request_data request)
{
	volatile accessible_data_entry found;
	if (!find_data(request.id, &found))
	{

		send_response(100, request.unique_id);
		return;
	}

	const_string name = found.name;
	void* data_ptr = found.data_ptr;
	size_t data_size = found.data_size;
	send_response_with_data(0, request.unique_id, name, data_ptr, data_size);
	return;
}

void request_put(request_data request)
{
	accessible_data_entry registered_data;
	if (!find_data(request.id, &registered_data))
	{
		write_message("cannot find the data.");
		send_response(100, request.unique_id);
		return;
	}

	if (registered_data.is_read_only)
	{
		write_message("the data is readonly.");
		send_response(100, request.unique_id);
		return;
	}

	uint32_t decoded_size = decode_base64(request.data, NULL);
	uint32_t size = registered_data.data_size;
	if (size != decoded_size)
	{
		write_message("don't match the data size.");
		send_response(100, request.unique_id);
		return;
	}
	if (size > MAX_COMMAND_DATA_SIZE)
	{
		write_message("the data size is too long.");
		send_response(100, request.unique_id);
		return;
	}

	decode_base64_s(request.data, request.data, MAX_COMMAND_DATA_SIZE);

	const uint32_t sum = adler32(request.data, decoded_size);

	if (sum != request.checksum)
	{
		write_message("don't match the check sum.");
		send_response(100, request.unique_id);
		return;
	}

	memcpy((uint8_t*) registered_data.data_ptr, request.data, size);

	send_response(0, request.unique_id);
	return;
}

void request_exec(request_data request)
{
	if (strcmp(request.id, "save-settings") == 0)
	{
		volatile size_t i = 0;
		for (i = 0; i < NUM_EEPRON_ENTRIES; i++)
		{
			uint8_t* address = (uint8_t*) eeprom_entries[i].offset;
			uint8_t* data = eeprom_entries[i].data;
			size_t size = eeprom_entries[i].size;

			if (eeprom_write(address, data, size) == EFAULT)
			{
				send_response(100, request.unique_id);
				return;
			}
		}
	}
	else if (strcmp(request.id, "load-settings") == 0)
	{
		volatile size_t i = 0;
		for (i = 0; i < NUM_EEPRON_ENTRIES; i++)
		{
			uint8_t* address = (uint8_t*) eeprom_entries[i].offset;
			uint8_t* data = eeprom_entries[i].data;
			size_t size = eeprom_entries[i].size;

			if (eeprom_read(address, data, size) == EFAULT)
			{
				send_response(100, request.unique_id);
				return;
			}
		}
	}

	send_response(0, request.unique_id);
	return;
}

static jsmntok_t* parse_json_object_as_request(char* js, jsmntok_t* tokens,
		size_t num_tokens, request_data* request)
{
	jsmntok_t* tok = tokens;
	const jsmntok_t* tok_end = tok + num_tokens;

	for (; tok < tok_end;)
	{
		const char* name = js + tok->start;
		js[tok->end] = '\0';
		tok++;

		const char* value = js + tok->start;
		js[tok->end] = '\0';
		tok++;

		if (strcmp(name, "method") == 0)
		{
			request->method = value;
		}
		else if (strcmp(name, "unique-id") == 0)
		{
			request->unique_id = str_to_uint32(value);
		}
		else if (strcmp(name, "id") == 0)
		{
			request->id = value;
		}
		else if (strcmp(name, "size") == 0)
		{
			request->size = str_to_uint32(value);
		}
		else if (strcmp(name, "data") == 0)
		{
			request->data = value;
		}
		else if (strcmp(name, "checksum") == 0)
		{
			request->checksum = str_to_uint32(value);
		}
	}

	return tok;
}

void command_request(command_data* cmd_data)
{
#define JSON_NUM_TOKENS 16

	static jsmn_parser json_parser;
	static jsmntok_t tokens[JSON_NUM_TOKENS];

	char* js = cmd_data->data;

	jsmn_init(&json_parser);
	jsmnerr_t error = jsmn_parse(&json_parser, js, tokens, JSON_NUM_TOKENS);

	if (error != JSMN_SUCCESS)
	{
		write_message("not sufficient space in tokens buffer.");
		return;
	}

	jsmntok_t* tok = tokens;

	int i = 0;
	if (tok->type != JSMN_OBJECT)
	{
		return;
	}

	request_data request =
	{ 0 };

	tok = parse_json_object_as_request(js, tok + 1, tok->size, &request);

	if (strcmp(request.method, "get") == 0)
	{
		request_get(request);
		return;
	}
	else if (strcmp(request.method, "put") == 0)
	{
		request_put(request);
		return;
	}
	else if (strcmp(request.method, "exec") == 0)
	{
		request_exec(request);
		return;
	}
}

/*---------------------------------------------------------------------------*/

static void monitor_switch(void)
{
// send switch states to ecu pin
	io_info.switches[0] = (LPC_GPIO->PIN[1] & _BV(4)) == 0;
	io_info.switches[1] = (LPC_GPIO->PIN[1] & _BV(27)) == 0;
	io_info.switches[2] = (LPC_GPIO->PIN[1] & _BV(26)) == 0;

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
	volatile uint32_t angle = ((1800 * fi_engine_state.th) >> 10);
	volatile uint32_t match = ((SystemCoreClock / 1000000) * (angle + 500));
	timer32_set_match(1, 1, match);
}

/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/

void init_cli(void)
{
	const uint32_t USART_BAUDRATE = 115200;
	usart_init(USART_BAUDRATE);
	usart_handler_init();

	initialize_command_system(command_error_func);

	uint32_t registered_data_count = sizeof(register_data_table)
			/ sizeof(register_data_table[0]);
	register_data(register_data_table, registered_data_count);
	register_command("request", command_request);
}

/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------------*/

/* Priorities at which the tasks are created. */
#define RXTX_ENGINE_DATA_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define RXTX_ENGINE_DATA_TASK_FREQENCY_MS	( 100 / portTICK_RATE_MS )

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

/*---------------------------------------------------------------------------*/

static void start_tasks(void)
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

	usart_mutex = xSemaphoreCreateMutex();

	if (usart_mutex == NULL)
	{
		write_error("cannot create \"usart_mutex\" mutex.", result);
	}

	/* Start the tasks running. */
	vTaskStartScheduler();
}

/*---------------------------------------------------------------------------*/

static void init_device_info(void)
{
	device_info.major_version = 1;
	device_info.minor_version = 0;
	device_info.injection_map_rev_points = 13;
	device_info.injection_map_th_points = 11;
}

static void init_servo_pulse(void)
{
	timer32_init(1, SystemCoreClock / 1000000 * 15000);
	timer32_set_pwm(1, SystemCoreClock / 1000000 * 15000);
//timer32_set_match(1, 1, SystemCoreClock * 7 / 10000);
	timer32_enable(1);
}

static void init_fi_settings(void)
{
	volatile uint32_t offset = 0;
	volatile size_t i = 0;

	for (i = 0; i < NUM_EEPRON_ENTRIES; i++)
	{
		eeprom_entries[i].offset = offset;
		offset += eeprom_entries[i].size;
	}

	for (i = 0; i < NUM_EEPRON_ENTRIES; i++)
	{
		uint8_t* address = (uint8_t*) eeprom_entries[i].offset;
		uint8_t* data = eeprom_entries[i].data;
		size_t size = eeprom_entries[i].size;

		eeprom_read(address, data, size);
	}
}

int main(void)
{
	init_fi_settings();

	init_io();

	init_device_info();

	init_cli();

	init_servo_pulse();

	start_tasks();

	for (;;)
		;

	return 0;
}
