/*
 * usart_util.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include "../system/peripheral/usart.h"
#include "../integer.h"
#include "usart_util.h"

static volatile uint32_t is_start_of_txt = 1;

void usart_write_char(uint8_t c)
{
	if (is_start_of_txt)
	{
		is_start_of_txt = 0;
		usart_write(USART_STX);
	}
	if (c == USART_ETX)
	{
		is_start_of_txt = 1;
	}

	usart_write(c);
}

uint32_t usart_writeln_string(const_string s)
{
	volatile uint32_t n;
	n = usart_write_string(s);
	usart_endln();
	return n;
}

uint32_t usart_write_string(const_string s)
{
	volatile uint32_t n;
	for (n = 0; s[n] != '\0'; n++)
	{
		usart_write_char(s[n]);
	}
	return n;
}

void usart_endln(void)
{
	usart_write_char(USART_ETX);
}

static uint8_t usart_util_conv_buf[12];

void usart_writeln_int32(int32_t value)
{
	int32_to_str(value, usart_util_conv_buf);
	usart_writeln_string(usart_util_conv_buf);
}

void usart_writeln_uint32(uint32_t value)
{
	uint32_to_str(value, usart_util_conv_buf);
	usart_writeln_string(usart_util_conv_buf);
}

void usart_writeln_uint32_hex(uint32_t value)
{
	uint32_to_hex_str(value, usart_util_conv_buf);
	usart_writeln_string(usart_util_conv_buf);
}

void usart_write_int32(int32_t value)
{
	int32_to_str(value, usart_util_conv_buf);
	usart_write_string(usart_util_conv_buf);
}

void usart_write_uint32(uint32_t value)
{
	uint32_to_str(value, usart_util_conv_buf);
	usart_write_string(usart_util_conv_buf);
}

void usart_write_uint8_hex(uint32_t value)
{
	uint8_to_hex_str(value, usart_util_conv_buf);
	usart_write_string(usart_util_conv_buf);
}

void usart_write_uint32_hex(uint32_t value)
{
	uint32_to_hex_str(value, usart_util_conv_buf);
	usart_write_string(usart_util_conv_buf);
}
