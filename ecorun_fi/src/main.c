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

// vattery
volatile uint32_t sum_mvoltage_adc = 0;
volatile uint32_t mvoltage_samples = 0;

// engine oil temperature
volatile uint32_t sum_oil_temperature_adc = 0;
volatile uint32_t oil_temperature_samples = 0;
volatile uint32_t ave_oil_temperature_registance = 0;

void timer32_1_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		adc_burst_read();/*
		 adc_read(1);
		 eg_data.th = adc_read(2);
		 adc_read(3);
		 adc_read(4);
		 adc_read(5);*/
	}
}
static volatile uint8_t ssp_buffer[2];
void timer32_0_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		// vattery
		if (mvoltage_samples != 0)
		{
			uint32_t ave_mvoltage_adc = sum_mvoltage_adc / mvoltage_samples;

			uint32_t ave_mvoltage = ave_mvoltage_adc * 18810; // ave_mvoltage_adc * 3300 / 1023 * R12 / R1
			cr_data.vattery_voltage = ave_mvoltage >> 12;
		}
		sum_mvoltage_adc = 0;
		mvoltage_samples = 0;

		// engine oil temperature
		if (oil_temperature_samples != 0)
		{
			uint32_t ave_temp_adc = sum_oil_temperature_adc / oil_temperature_samples;

			ave_oil_temperature_registance = ave_temp_adc * 1000 / (4096 - ave_temp_adc);
			eg_data.oil_temp = ave_oil_temperature_registance;
		}
		sum_oil_temperature_adc = 0;
		oil_temperature_samples = 0;
	}
}

void command_error_func(const char* id, command_func func)
{
	usart_write_string("msg <can't register command : ");
	usart_write_string(id);
	usart_writeln_string(">");
}

static const named_data register_data_table[] =
{
{ "engine_data", (void*) &eg_data, sizeof(eg_data), true },
{ "basic_inject_time_map", (void*) &fi_settings.basic_inject_time_map[0][0], sizeof(fi_settings.basic_inject_time_map), false },
{ "car_data", (void*) &cr_data, sizeof(cr_data), true } };

void command_get(command_data* data)
{
	const_string id = data->args[0].arg_value;
	find_and_put_data(id);
}

volatile uint8_t transaction_buf[512];

static inline void write_message(const_string msg)
{
	usart_write_string("msg <");
	usart_write_string(msg);
	usart_writeln_string(">");
}

void command_put(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	const uint32_t encoded_size = str_to_uint32(data->args[1].arg_value);
	const_string encoded_data = data->args[2].arg_value;
	const uint32_t sum = str_to_uint32(data->args[3].arg_value);

	named_data registered_data;
	if (find_data(id, &registered_data))
	{
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
		if (size > sizeof(transaction_buf))
		{
			write_message("the data size is too long.");
			return;
		}
		decode_base64_s(encoded_data, (uint8_t*) transaction_buf, 512);

		uint32_t check_sum = adler32(transaction_buf, decoded_size);

		if (check_sum != sum)
		{
			write_message("don't match the check sum.");
			return;
		}

		memcpy((uint8_t*) registered_data.data_ptr, transaction_buf, size);
	}
}

void adc_handler(uint8_t num, uint32_t value)
{
	switch (num)
	{
	case 1:
		sum_mvoltage_adc += value;
		mvoltage_samples++;
		break;
	case 2:
		eg_data.th = value;
		break;
	case 4:
		sum_oil_temperature_adc += value;
		oil_temperature_samples++;
		break;
	case 5:
		eg_data.vacuum = value;
		break;
	}
}

int main(void)
{
	SystemCoreClockUpdate();

	const uint32_t UART_BAUDRATE = 115200;
	fi_set_default();
	usart_init(UART_BAUDRATE);
	//ssp_init();

	timer32_init(0, SystemCoreClock);
	timer32_add_event(0, timer32_0_handler);
	timer32_enable(0);
	timer32_init(1, SystemCoreClock / 100);
	timer32_add_event(1, timer32_1_handler);
	timer32_enable(1);
	adc_init(ADC_CLK);
	adc_add_event(adc_handler);
	systimer_init();

	initialize_command_system(command_error_func);

	uint32_t registered_data_count = sizeof(register_data_table) / sizeof(register_data_table[0]);
	register_data(register_data_table, registered_data_count);
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

	LPC_IOCON->PIO0_2 = 0x10;
	LPC_IOCON->PIO1_10 = 0x10;
	LPC_IOCON->PIO1_26 = 0x10;
	LPC_IOCON->PIO1_27 = 0x10;
	LPC_IOCON->PIO1_4 = 0x10;

	LPC_GPIO->DIR[0] &= ~_BV(2);
	LPC_GPIO->DIR[1] &= ~_BV(10);
	LPC_GPIO->DIR[1] &= ~_BV(26);
	LPC_GPIO->DIR[1] &= ~_BV(27);
	LPC_GPIO->DIR[1] &= ~_BV(4);

	LPC_IOCON->PIO1_0 = 0x10;
	LPC_IOCON->PIO1_25 = 0x10;
	LPC_IOCON->PIO1_19 = 0x10;
	LPC_IOCON->PIO1_7 = 0x10;

	LPC_GPIO->DIR[1] |= _BV(0);
	LPC_GPIO->DIR[1] |= _BV(25);
	LPC_GPIO->DIR[1] |= _BV(19);
	LPC_GPIO->DIR[1] |= _BV(7);

	LPC_GPIO->SET[1] |= _BV(0);
	LPC_GPIO->SET[1] |= _BV(25);
	LPC_GPIO->SET[1] |= _BV(19);
	LPC_GPIO->SET[1] |= _BV(7);

	while (1)
	{
		execute_all_command();
	}

	return 0;
}
