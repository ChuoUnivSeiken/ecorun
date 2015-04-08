/*
 * adler32.c
 *
 *  Created on: 2014/08/08
 *      Author: Yoshio
 */

#include "adler32.h"

#define MOD_ADLER 65521

uint32_t adler32(const_buffer data, size_t len)
{
	volatile uint32_t a = 1, b = 0;

	while (len > 0)
	{
		size_t tlen = len > 5550 ? 5550 : len;
		len -= tlen;
		do
		{
			a += *data++;
			b += a;
		} while (--tlen);

		a %= MOD_ADLER;
		b %= MOD_ADLER;
	}

	return (b << 16) | a;
}
