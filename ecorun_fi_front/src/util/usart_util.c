/*
 * usart_util.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include "../system/peripheral/usart.h"
#include "../integer.h"
#include "usart_util.h"

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
