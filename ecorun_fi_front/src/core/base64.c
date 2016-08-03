/*
 * base64.c
 *
 *  Created on: 2014/08/27
 *      Author: Yoshio
 */

#include <stddef.h>
#include <string.h>
#include "base64.h"
#include "../system/peripheral/usart.h"

static const uint8_t b64[] =
{ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		62 /* + */, 128, 128, 128, 63 /*/ */, 52 /* 0 */, 53, 54, 55, 56, 57,
		58, 59, 60, 61 /* 9 */, 128, 128, 128, 0 /* = */, 128, 128, 128,
		0 /* A */, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25 /* Z */, 128, 128, 128, 128, 128, 128,
		26 /* a */, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
		42, 43, 44, 45, 46, 47, 48, 49, 50, 51 /* z */};
static const uint8_t* w =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void usart_write_base64(const_buffer data, uint32_t size)
{
	volatile uint32_t encoded_size = 0;
	volatile const_buffer p = data;
	volatile uint32_t x = 0, l = 0;

	for (; size-- > 0; p++)
	{
		x = x << 8 | *p;
		for (l += 8; l >= 6; l -= 6)
		{
			usart_write_char(w[(x >> (l - 6)) & 0x3f]);
			encoded_size++;
		}
	}
	if (l > 0)
	{
		x <<= 6 - l;
		usart_write_char(w[x & 0x3f]);
		encoded_size++;
	}
	for (; encoded_size & 3;)
	{
		usart_write_char('=');
		encoded_size++;
	}
}

uint32_t encode_base64(const_buffer data, uint32_t size, string dest)
{
	uint32_t i; // sizeof encoded char
	if (dest == NULL)
	{
		return (8 * size + 5) / 6;
	}
	const_buffer p = data;
	int x = 0, l = 0;
	i = 0;

	for (; size-- > 0; p++)
	{
		x = x << 8 | *p;
		for (l += 8; l >= 6; l -= 6)
		{
			dest[i++] = w[(x >> (l - 6)) & 0x3f];
		}
	}
	if (l > 0)
	{
		x <<= 6 - l;
		dest[i++] = w[x & 0x3f];
	}
	for (; i & 3;)
	{
		dest[i++] = '=';
	}
	dest[i] = '\0';
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
uint32_t decode_base64_s(const_string base64, buffer dest, uint32_t dest_size)
{
	buffer dest_ptr = dest;
	uint32_t base64_size = strlen(base64);
	char temp[4];
	char temp2[4];
	int i = 0, j;
	uint32_t len = 0;

	if (dest == NULL)
	{
		len = (base64_size * 3) / 4;
		if (base64[base64_size - 1] == '=')
		{
			if (base64[base64_size - 2] == '=') /* = がつくのは最大2個 */
			{
				return len - 2;
			}
			return len - 1;
		}
	}

	while (base64_size > 0)
	{
		for (j = 0; (j < 4) & (base64_size > 0); j++)
		{
			temp[j] = b64[*(base64++)];
			temp2[j] = *base64;
			base64_size--;
		}
		for (j = 0; j < 3; j++)
		{
			if (temp2[j + 1] != '=' && i < dest_size)
			{
				dest_ptr[i++] = temp[j] << ((j << 1) + 2)
						| temp[j + 1] >> (((2 - j) << 1));

			}
		}
	}

	return i;
}

uint32_t decode_base64(const_string base64, buffer dest)
{
	buffer dest_ptr = dest;
	char temp[4];
	char temp2[4];
	int i = 0, j;
	uint32_t base64_len = strlen(base64);

	if (dest == NULL)
	{
		uint32_t len = (base64_len * 3) / 4;
		if (base64[base64_len - 1] == '=')
		{
			if (base64[base64_len - 2] == '=')
			{
				return len - 2;
			}
			return len - 1;
		}
	}

	while (base64_len > 0)
	{
		for (j = 0; (j < 4) & (base64_len > 0); j++)
		{
			temp[j] = b64[*(base64++)];
			temp2[j] = *base64;
			base64_len--;
		}
		for (j = 0; j < 3; j++)
		{
			if (temp[j] != '=')
			{
				dest_ptr[i++] = temp[j] << ((j << 1) + 2)
						| temp[j + 1] >> (((2 - j) << 1));
			}
		}
	}

	return i;
}
