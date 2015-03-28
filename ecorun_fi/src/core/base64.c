/*
 * base64.c
 *
 *  Created on: 2014/08/27
 *      Author: Yoshio
 */

#include <stddef.h>
#include "base64.h"
#include "../system/peripheral/usart.h"

void usart_write_base64(const uint8_t* data, uint32_t size)
{
	uint32_t i, ensize = (8 * size + 5) / 6; // sizeof encoded char
	//uart_uint32(ensize);
	//uart_puts_with_term("\n");

	char *w = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	volatile uint8_t *p = data;
	int x = 0, l = 0;
	i = 0;

	for (; size-- > 0; p++)
	{
		x = x << 8 | *p;
		for (l += 8; l >= 6; l -= 6)
		{
			usart_write_char(w[(x >> (l - 6)) & 0x3f]);
			i++;
		}
	}
	if (l > 0)
	{
		x <<= 6 - l;
		usart_write_char(w[x & 0x3f]);
		i++;
	}
	for (; i & 3;)
	{
		usart_write_char('=');
		i++;
	}
}

uint32_t get_base64(const uint8_t* data, uint32_t size, uint8_t* base64)
{
	uint32_t i, ensize; // sizeof encoded char
	if (base64 == NULL)
	{
		ensize = (8 * size + 5) / 6;
		return ensize;
	}

	char *w = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	volatile uint8_t *p = data;
	int x = 0, l = 0;
	i = 0;

	for (; size-- > 0; p++)
	{
		x = x << 8 | *p;
		for (l += 8; l >= 6; l -= 6)
		{
			base64[i++] = w[(x >> (l - 6)) & 0x3f];
		}
	}
	if (l > 0)
	{
		x <<= 6 - l;
		base64[i++] = w[x & 0x3f];
	}
	for (; i & 3;)
	{
		base64[i++] = '=';
	}
	base64[i] = '\0';
	return i;
}

/**
 * Base64 エンコードされた文字列をデコードします。
 * @param base64 Base64 エンコードされた文字列
 * @param base64　base64文字列の長さ
 * @param destination デコードされた文字列が出力される配列
 * @return 出力される文字列の長さ。
 *　@remarks 失敗した場合は戻り値は0。
 */
uint32_t decode_base64(const uint8_t* base64, uint32_t base64_size, uint8_t* destination)
{
	char b64[128], *w = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	char *p, c[4], *buff = destination;
	int i = 0, j;

	p = base64;

	for (j = 0; j < 65; j++)
		b64[w[j]] = j & 63;
	while (base64_size > 0)
	{
		for (j = 0; j < 4; j++)
		{
			c[j] = b64[*(p++)];
			base64_size--;
		}
		for (j = 0; j < 3; j++)
			buff[i++] = c[j] << ((j << 1) + 2) | c[j + 1] >> (((2 - j) << 1));
	}
	buff[i] = '\0';

	return i;
}
