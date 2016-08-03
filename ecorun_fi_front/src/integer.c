/*
 * Copyright (c) 2015 Yoshio Nakamura
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ソースコード形式かバイナリ形式か、変更するかしないかを問わず、以下の条件を満たす場合に限り、再頒布および使用が許可されます。
 *
 * ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、および下記免責条項を含めること。
 * バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、上記の著作権表示、本条件一覧、および下記免責条項を含めること。
 * 書面による特別の許可なしに、本ソフトウェアから派生した製品の宣伝または販売促進に、<組織>の名前またはコントリビューターの名前を使用してはならない。
 * 本ソフトウェアは、著作権者およびコントリビューターによって「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、および特定の目的に対する適合性に関する暗黙の保証も含め、またそれに限定されない、いかなる保証もありません。著作権者もコントリビューターも、事由のいかんを問わず、 損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか（過失その他の）不法行為であるかを問わず、仮にそのような損害が発生する可能性を知らされていたとしても、本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、使用の喪失、データの喪失、利益の喪失、業務の中断も含め、またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */

#include "integer.h"
#include <string.h>

volatile static uint8_t num_convert_buf[10];

size_t int32_to_str(int32_t num, string str)
{
	volatile int count = 0;
	if (num < 0)
	{
		str[count++] = '-';
		num = -num;
	}

	count += uint32_to_str(num, &str[count]);

	return count;
}

size_t uint32_to_str(uint32_t num, string str)
{
	volatile int bufpos = 0;
	volatile int count = 0;
	if (num == 0)
	{
		strcpy(str, "0");
		return 1;
	}

	while (num != 0)
	{
		num_convert_buf[bufpos++] = (num % 10) + '0';
		num = num / 10;
	}

	while (--bufpos >= 0)
	{
		str[count++] = num_convert_buf[bufpos];
	}
	str[count] = '\0';

	return count;
}

uint8_t hex_table[16] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
				'e', 'f' };

size_t uint8_to_hex_str(uint32_t num, string str)
{
	volatile uint32_t bufpos = 0;
	volatile uint32_t count = 0;
	strcpy(str, "0x");
	count = count + 2;

	while (bufpos < 2)
	{
		num_convert_buf[bufpos++] = hex_table[(num & 0x0f)];
		num = (num >> 4);
	}

	while (bufpos-- > 0)
	{
		str[count++] = num_convert_buf[bufpos];
	}
	str[count] = '\0';

	return count;
}

size_t uint32_to_hex_str(uint32_t num, string str)
{
	volatile uint32_t bufpos = 0;
	volatile uint32_t count = 0;
	strcpy(str, "0x");
	count = count + 2;

	while (bufpos < 8)
	{
		num_convert_buf[bufpos++] = hex_table[(num & 0x0f)];
		num = (num >> 4);
	}

	while (bufpos-- > 0)
	{
		str[count++] = num_convert_buf[bufpos];
	}
	str[count] = '\0';

	return count;
}

size_t str_to_uint32(const_string str)
{
	volatile uint32_t num = 0;
	volatile uint32_t place = 1;
	volatile uint32_t base = 10;
	volatile const_string ptr = str + strlen(str);

	if (str[0] == '0')
	{
		switch (str[1])
		{
		case 'x':
		case 'X':
			base = 16;
			break;
		default:
			base = 8;
			break;
		}
	}

	while (ptr-- > str)
	{
		uint8_t ch = *ptr;
		uint32_t val = 0;
		if ('0' <= ch && ch <= '9')
		{
			val = ch - '0';
		}
		else if ('a' <= ch && ch <= 'f')
		{
			val = ch - 'a' + 9;
		}
		else if ('A' <= ch && ch <= 'F')
		{
			val = ch - 'A' + 9;
		}

		num += (val * place);
		place *= base;
	}
	return num;
}

