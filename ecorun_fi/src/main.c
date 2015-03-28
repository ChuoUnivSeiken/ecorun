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

static uint8_t send_buf[128]; // TODO check buffer size

void put_data(uint8_t* data, uint32_t size, const char* id)
{
	usart_write_string("put ");
	memcpy(send_buf, id, 4);
	memcpy(send_buf + 4, (const uint8_t*) data, size);
	uint32_t check_sum = adler32((const uint8_t*) send_buf, size + 4);
	memcpy(send_buf + 4 + size, (const uint8_t*) &check_sum, 4);
	usart_write_base64((uint8_t*) send_buf, size + 8);
	usart_writeln_string("");
}

void command_get(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	if (strncmp(id, "ENGN", 4) == 0)
	{
		eg_data.is_af_rich = 1;
		eg_data.is_fuel_cut = 2;
		eg_data.oil_temp = 3;
		eg_data.rev = 4;
		eg_data.th = 5;
		eg_data.current_inject_ended_count = 6;
		eg_data.current_inject_started_count = 7;
		eg_data.current_total_injected_time = 8;
		put_data((uint8_t*) &eg_data, sizeof(eg_data), "ENGN");
	}
	else if (strncmp(id, "CARD", 4) == 0)
	{
		cr_data.vattery_voltage = 1;
		cr_data.wheel_count = 2;
		cr_data.wheel_rotation_period = 3;
		put_data((uint8_t*) &cr_data, sizeof(cr_data), "CARD");
	}
}

void command_put(command_data* data)
{

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

	while (1)
	{
		execute_all_command();
	}

	return 0;
}
