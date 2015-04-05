/*
 * main.c
 *
 *  Created on: 2015/03/12
 *      Author: Yoshio
 */

#include "system/cmsis/LPC13Uxx.h"
#include "system/peripheral/usart.h"
#include "system/peripheral/timer.h"
#include "util/usart_util.h"
#include "core/command.h"
#include "core/base64.h"
#include "core/adler32.h"
#include "carsystem/car_info.h"
#include "carsystem/injection.h"
#include "carsystem/fi_settings.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define _BV(n) ((uint32_t)0x1 << n)

void timer32_1_handler(uint8_t timer, uint8_t num)
{
}

void command_error_func(const char* id, command_func func)
{
	usart_write_string("message Can't register command : ");
	usart_write_string(id);
	usart_writeln_string("\n");
}

static uint8_t buf[128]; // TODO check buffer size

/**
 * @remarks put (data)
 * 			data = (id 4bytes)(space 1byte)(base64 encoded binary nbytes)(checksum 4bytes)
 */
void put_data(const uint8_t* data, uint32_t size, const char* id)
{
	usart_write_string("put ");
	usart_write_string(id);
	usart_write_string(" ");
	usart_writeln_uint32(size);
	usart_write_string(" ");
	usart_write_base64(data, size);
	usart_write_string(" ");
	usart_writeln_uint32_hex(adler32(data, size));
	usart_writeln_string("");
}

typedef struct named_variable_t
{
	const uint8_t* name;
	void* variable;
	uint32_t variable_size;
} named_variable;

static named_variable registered_variables[] =
{
{ "engine_data", &eg_data, sizeof(engine_data) },
{ "car_data", &cr_data, sizeof(car_data) } };
static uint32_t registered_variables_count = sizeof(registered_variables) / sizeof(registered_variables[0]);

void command_get(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	volatile uint32_t i = 0;
	for (i = 0; i < registered_variables_count; i++)
	{
		if (strcmp(id, registered_variables[i].name) == 0)
		{
			put_data((uint8_t*) registered_variables[i].variable, registered_variables[i].variable_size, registered_variables[i].name);
		}
	}
}

void command_put(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	const uint32_t size = str_to_uint32(data->args[1].arg_value);
	const uint8_t* bin_data = str_to_uint32(data->args[2].arg_value);
	const uint32_t bin_data_size = str_to_uint32(data->args[2].arg_value);
	volatile uint32_t i = 0;
	for (i = 0; i < registered_variables_count; i++)
	{
		if (strcmp(id, registered_variables[i].name) == 0)
		{
			uint32_t b64_size = ((sizeof(engine_data) + 4) * 8 + 5) / 6;
			b64_size = ((b64_size << 2) + 3) >> 2;
			// assert(size == b64_size);
			decode_base64(bin_data, bin_data_size, (uint8_t*) registered_variables[i].variable, registered_variables[i].variable_size);
		}
	}
}

int main(void)
{
	SystemCoreClockUpdate();

	const uint32_t UART_BAUDRATE = 115200;

	usart_init(UART_BAUDRATE);
	timer32_init(1, SystemCoreClock / 2);
	timer32_add_event(1, timer32_1_handler);
	timer32_enable(1);

	initialize_command_system(command_error_func);

	register_command("get", command_get);
	register_command("put", command_put);

	eg_data.is_af_rich = 1;
	eg_data.is_fuel_cut = 2;
	eg_data.oil_temp = 3;
	eg_data.rev = 4;
	eg_data.th = 5;
	eg_data.current_inject_ended_count = 6;
	eg_data.current_inject_started_count = 7;
	eg_data.current_total_injected_time = 8;

	cr_data.vattery_voltage = 1;
	cr_data.wheel_count = 2;
	cr_data.wheel_rotation_period = 3;

	while (1)
	{
		execute_all_command();
	}

	return 0;
}
