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
#include "system/systimer.h"
#include "util/usart_util.h"
#include "core/command.h"
#include "core/base64.h"
#include "core/adler32.h"
#include "integer.h"
#include "carsystem/car_info.h"
#include "carsystem/injection.h"
#include "carsystem/fi_settings.h"
#include <string.h>

void timer32_1_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		adc_read(0);
	}
}

void command_error_func(const char* id, command_func func)
{
	usart_write_string("msg <can't register command : ");
	usart_write_string(id);
	usart_writeln_string(">");
}

/**
 * @remarks put (data)
 * 			data = (id 4bytes)(space 1byte)(base64 encoded binary nbytes)(checksum 4bytes)
 */
void put_data(const uint8_t* data, uint32_t size, const char* id)
{
	systime_t time = systimer_tick();
	usart_write_string("put ");
	usart_write_string(id);
	usart_write_string(" ");
	usart_write_uint32(size);
	usart_write_string(" ");
	usart_write_base64(data, size);
	usart_write_string(" ");
	usart_write_uint32_hex(adler32(data, size));
	usart_write_string(" ");
	usart_write_uint32(time.seconds);
	usart_write_string(" ");
	usart_write_uint32(time.counts);
	usart_writeln_string("");
}

typedef struct named_data_t
{
	const uint8_t* name;
	void* data_ptr;
	uint32_t data_size;
	bool is_read_only;
}volatile named_data;

static volatile named_data registered_data[] =
{
{ "engine_data", &eg_data, sizeof(engine_data), true },
{ "car_data", &cr_data, sizeof(car_data), true } };
static uint32_t registered_data_count = sizeof(registered_data)
		/ sizeof(registered_data[0]);

void command_get(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	volatile uint32_t i = 0;
	for (i = 0; i < registered_data_count; i++)
	{
		if (strcmp(id, registered_data[i].name) == 0)
		{
			put_data((uint8_t*) registered_data[i].data_ptr,
					registered_data[i].data_size, registered_data[i].name);
		}
	}
}

uint8_t buf[128];

void command_put(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	const uint32_t encoded_size = str_to_uint32(data->args[1].arg_value);
	const uint8_t* encoded_data = data->args[2].arg_value;
	const uint32_t sum = str_to_uint32(data->args[3].arg_value);
	volatile uint32_t i = 0;
	for (i = 0; i < registered_data_count; i++)
	{
		if (strcmp(id, registered_data[i].name) == 0)
		{
			if (registered_data[i].is_read_only)
			{
				usart_writeln_string("msg <the data is readonly.>");
				return;
			}
			uint32_t decoded_size = decode_base64(encoded_data, NULL);
			uint32_t size = registered_data[i].data_size;
			if (size != decoded_size)
			{
				usart_writeln_string("msg <don't match the data size.>");
				return;
			}
			decode_base64_s(encoded_data, strlen(encoded_data), (uint8_t*) buf,
					size);

			uint32_t check_sum = adler32(buf, decoded_size);

			if (check_sum != sum)
			{
				usart_writeln_string("msg <don't match the check sum.>");
				return;
			}

			memcpy((uint8_t*) registered_data[i].data_ptr, buf,
					registered_data[i].data_size);

		}
	}
}

int main(void)
{
	SystemCoreClockUpdate();

	const uint32_t UART_BAUDRATE = 115200;

	usart_init(UART_BAUDRATE);
	timer32_init(1, SystemCoreClock / 100);
	timer32_add_event(1, timer32_1_handler);
	timer32_enable(1);
	adc_init(ADC_CLK);
	systimer_init();

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
