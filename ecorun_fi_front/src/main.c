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

// vattery
volatile uint32_t sum_mvoltage_adc = 0;
volatile uint32_t mvoltage_samples = 0;

// engine oil temperature
volatile uint32_t sum_oil_temperature_adc = 0;
volatile uint32_t oil_temperature_samples = 0;
volatile uint32_t ave_oil_temperature_registance = 0;

volatile uint8_t ssp_buf[sizeof(eg_data) + 4];

void receive_eneine_data(void)
{
	volatile uint32_t i = 0;

	// receive engine data
	{
		volatile uint32_t size = sizeof(eg_data) + 4;
		volatile uint32_t addr = 0x00;
		volatile uint32_t func = 0x00;
		volatile uint16_t data = ((func << 15) | (addr << 8) | (size & 0xFF));
		ssp_send2(&data, 1);
		for (i = 0; i < size; i++)
		{
			volatile uint8_t received = 0;
			ssp_receive(&received, 1);

			ssp_buf[i] = received;
		}

		volatile uint32_t checksum = *(uint32_t*) ((uint8_t*) ssp_buf
				+ sizeof(eg_data));

		volatile uint32_t sum = adler32(ssp_buf, sizeof(eg_data));

		if (sum == checksum)
		{
			memcpy(&eg_data, ssp_buf, sizeof(eg_data));
		}
	}
	// send injection map
	{
		fi_settings.checksum = adler32(fi_settings.basic_inject_time_map,
				sizeof(fi_settings.basic_inject_time_map));
		volatile uint8_t* fi_settings_ptr = (uint8_t*) &fi_settings;

		volatile uint32_t size = sizeof(fi_settings.basic_inject_time_map) + 4;
		volatile uint32_t addr = 0x01;
		volatile uint32_t func = 0x01;
		volatile uint16_t data = ((func << 15) | (addr << 8) | 68);
		ssp_send2(&data, 1);
		for (i = 0; i < size; i++)
		{
			volatile uint8_t send = fi_settings_ptr[i];
			ssp_send(&send, 1);
		}
	}
}

void timer32_0_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		receive_eneine_data();
	}
}

void timer32_1_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
	}
}

void timer16_0_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
	}
}

void timer16_1_handler(uint8_t timer, uint8_t num)
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
			uint32_t ave_temp_adc = sum_oil_temperature_adc
					/ oil_temperature_samples;

			ave_oil_temperature_registance = ave_temp_adc * 1000
					/ (4096 - ave_temp_adc);
			eg_data.oil_temp = ave_oil_temperature_registance;
		}
		sum_oil_temperature_adc = 0;
		oil_temperature_samples = 0;
		/*
		 timer32_set_match(1, 1,
		 SystemCoreClock / 70 * (0 + (4096 - eg_data.th) * 144 / 4096)
		 / 144);*/

		LPC_GPIO->PIN[1] ^= _BV(24);
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
{ "basic_inject_time_map", (void*) &fi_settings.basic_inject_time_map[0][0],
		sizeof(fi_settings.basic_inject_time_map), false },
{ "car_data", (void*) &cr_data, sizeof(cr_data), true } };

void command_get(command_data* data)
{
	const_string id = data->args[0].arg_value;

	__disable_irq();
	find_and_put_data(id);
	__enable_irq();
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

void command_exec(command_data* data)
{
	const uint8_t* id = data->args[0].arg_value;
	if (strcmp(id, "save-settings") == 0)
	{
		eeprom_write((uint8_t*) 0, (buffer*) fi_settings.basic_inject_time_map,
				sizeof(fi_settings.basic_inject_time_map));

		write_message("settings saved.");
	}
	else if (strcmp(id, "load-settings") == 0)
	{
		eeprom_read((uint8_t*) 0, (buffer*) fi_settings.basic_inject_time_map,
				sizeof(fi_settings.basic_inject_time_map));

		write_message("settings loaded.");
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

#define USE_BLUETOOTH 1

#define SSP0_SCK0_1_29 0
#define SSP0_SCK0_0_10 1
#define SSP0_SCK0_0_6 2
#define SSP0_SCK0_LOCATION SSP0_SCK0_0_10

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

void init_io_ssp(void)
{
	/* Set 0.8 to SSP MISO0 */
	LPC_IOCON->PIO0_8 &= ~0x07;
	LPC_IOCON->PIO0_8 |= 0x01;

	/* Set 0.9 to SSP MOSI0 */
	LPC_IOCON->PIO0_9 &= ~0x07;
	LPC_IOCON->PIO0_9 |= 0x01;

#if (SSP0_SCK0_LOCATION == SSP0_SCK0_1_29)
	/* Set 1.29 to SSP SCK0 (0.6 is often used by USB and 0.10 for SWD) */
	LPC_IOCON->PIO1_29 = 0x01;
#elif (SSP0_SCK0_LOCATION == SSP0_SCK0_0_10)
	/* Set 0.10 to SSP SCK0 (may be required for SWD!) */
	LPC_IOCON->SWCLK_PIO0_10 = 0x02;
#elif (SSP0_SCK0_LOCATION == SSP0_SCK0_0_6)
	/* Set 0.6 to SSP SCK0 (may be required for USB!) */
	LPC_IOCON->PIO0_6 = 0x02;
#else
#error "Invalid SSP0_SCK0_LOCATION"
#endif

	LPC_IOCON->PIO0_2 &= ~0x07;
	LPC_IOCON->PIO0_2 |= 0x01;
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
}

void init_io(void)
{
	//init_io_adc();
	init_io_ssp();
	init_io_usart();
	init_io_signals();
}

void init_cli(void)
{
	const uint32_t USART_BAUDRATE = 115200;
	usart_init(USART_BAUDRATE);

	initialize_command_system(command_error_func);

	uint32_t registered_data_count = sizeof(register_data_table)
			/ sizeof(register_data_table[0]);
	register_data(register_data_table, registered_data_count);
	register_command("get", command_get);
	register_command("put", command_put);
	register_command("exec", command_exec);
}

int main(void)
{
	SystemCoreClockUpdate();

	NVIC_SetPriority(USART_IRQn, 1);
	NVIC_SetPriority(CT32B0_IRQn, 2);

	eeprom_read((uint8_t*) 0, (buffer*) fi_settings.basic_inject_time_map,
			sizeof(fi_settings.basic_inject_time_map));

	init_io();

	//fi_set_default();

	timer16_init(0, 10000, SystemCoreClock / 10000 / 1000);
	timer16_add_event(0, timer16_0_handler);
	timer16_enable(0);

	LPC_IOCON->PIO1_11 = 0x10;
	LPC_GPIO->DIR[1] &= ~_BV(11);
	/*
	 timer16_init(1, 10000, SystemCoreClock / 10000);
	 timer16_set_match(1, 1, SystemCoreClock / 10000 / 2);
	 timer16_add_event(1, timer16_1_handler);
	 timer16_enable(1);
	 */
	/*
	 LPC_IOCON->PIO1_3 &= ~0x07;
	 LPC_IOCON->PIO1_3 |= 0x01;
	 LPC_GPIO->DIR[1] |= _BV(3);
	 adc_init(ADC_CLK);
	 adc_add_event(adc_handler);
	 */
	ssp_init();

	systimer_init();

	init_cli();

	timer32_init(0, SystemCoreClock / 20);
	timer32_add_event(0, timer32_0_handler);
	timer32_enable(0);

	while (1)
	{
		execute_all_command();
	}

	return 0;
}
