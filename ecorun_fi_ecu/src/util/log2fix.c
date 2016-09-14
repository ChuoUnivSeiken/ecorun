/*
 The MIT License (MIT)

 Copyright (c) 2015 Dan Moulding

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <stddef.h>

#include "log2fix.h"

#define INV_LOG2_E_Q1DOT31  UINT64_C(0x58b90bfc) // Inverse log base 2 of e
#define INV_LOG2_10_Q1DOT31 UINT64_C(0x268826a1) // Inverse log base 2 of 10

int32_t log2fix(uint32_t x, size_t precision)
{
	// This implementation is based on Clay. S. Turner's fast binary logarithm
	// algorithm[1].

	int32_t b = 1U << (precision - 1);
	int32_t y = 0;

	if (precision < 1 || precision > 31)
	{
		errno = EINVAL;
		return INT32_MAX; // indicates an error
	}

	if (x == 0)
	{
		return INT32_MIN; // represents negative infinity
	}

	while (x < 1U << precision)
	{
		x <<= 1;
		y -= 1U << precision;
	}

	while (x >= 2U << precision)
	{
		x >>= 1;
		y += 1U << precision;
	}

	uint64_t z = x;

	for (size_t i = 0; i < precision; i++)
	{
		z = z * z >> precision;
		if (z >= 2U << precision)
		{
			z >>= 1;
			y += b;
		}
		b >>= 1;
	}

	return y;
}

int32_t logfix(uint32_t x, size_t precision)
{
	uint64_t t;

	t = log2fix(x, precision) * INV_LOG2_E_Q1DOT31;

	return t >> 31;
}

int32_t log10fix(uint32_t x, size_t precision)
{
	uint64_t t;

	t = log2fix(x, precision) * INV_LOG2_10_Q1DOT31;

	return t >> 31;
}

// [1] C. S. Turner,  "A Fast Binary Logarithm Algorithm", IEEE Signal
//     Processing Mag., pp. 124,140, Sep. 2010.
