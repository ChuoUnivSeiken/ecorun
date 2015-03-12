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
 * �\�[�X�R�[�h�`�����o�C�i���`�����A�ύX���邩���Ȃ������킸�A�ȉ��̏����𖞂����ꍇ�Ɍ���A�ĔЕz����юg�p��������܂��B
 *
 * �\�[�X�R�[�h���ĔЕz����ꍇ�A��L�̒��쌠�\���A�{�����ꗗ�A����щ��L�Ɛӏ������܂߂邱�ƁB
 * �o�C�i���`���ōĔЕz����ꍇ�A�Еz���ɕt���̃h�L�������g���̎����ɁA��L�̒��쌠�\���A�{�����ꗗ�A����щ��L�Ɛӏ������܂߂邱�ƁB
 * ���ʂɂ����ʂ̋��Ȃ��ɁA�{�\�t�g�E�F�A����h���������i�̐�`�܂��͔̔����i�ɁA<�g�D>�̖��O�܂��̓R���g���r���[�^�[�̖��O���g�p���Ă͂Ȃ�Ȃ��B
 * �{�\�t�g�E�F�A�́A���쌠�҂���уR���g���r���[�^�[�ɂ���āu����̂܂܁v�񋟂���Ă���A�����َ����킸�A���ƓI�Ȏg�p�\���A����ѓ���̖ړI�ɑ΂���K�����Ɋւ���Öق̕ۏ؂��܂߁A�܂�����Ɍ��肳��Ȃ��A�����Ȃ�ۏ؂�����܂���B���쌠�҂��R���g���r���[�^�[���A���R�̂�������킸�A ���Q�����̌�����������킸�A���ӔC�̍������_��ł��邩���i�ӔC�ł��邩�i�ߎ����̑��́j�s�@�s�ׂł��邩���킸�A���ɂ��̂悤�ȑ��Q����������\����m�炳��Ă����Ƃ��Ă��A�{�\�t�g�E�F�A�̎g�p�ɂ���Ĕ��������i��֕i�܂��͑�p�T�[�r�X�̒��B�A�g�p�̑r���A�f�[�^�̑r���A���v�̑r���A�Ɩ��̒��f���܂߁A�܂�����Ɍ��肳��Ȃ��j���ڑ��Q�A�Ԑڑ��Q�A�����I�ȑ��Q�A���ʑ��Q�A�����I���Q�A�܂��͌��ʑ��Q�ɂ��āA��ؐӔC�𕉂�Ȃ����̂Ƃ��܂��B
 */

#include "integer.h"
#include <string.h>

volatile static char num_convert_buf[10];

uint32_t int32_to_str(int32_t num, char* buf)
{
	volatile int count = 0;
	if (num < 0)
	{
		buf[count++] = '-';
		num = -num;
	}

	count += uint32_to_str(num, &buf[count]);

	return count;
}

uint32_t uint32_to_str(uint32_t num, char* buf)
{
	volatile int bufpos = 0;
	volatile int count = 0;
	if (num == 0)
	{
		strcpy(buf, "0");
		return 1;
	}

	while (num != 0)
	{
		num_convert_buf[bufpos++] = (num % 10) + '0';
		num = num / 10;
	}

	while (--bufpos >= 0)
	{
		buf[count++] = num_convert_buf[bufpos];
	}
	buf[count++] = '\0';

	return count;
}

char hex_table[16] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
				'e', 'f' };

uint32_t uint32_to_hex_str(uint32_t num, char* buf)
{
	volatile int bufpos = 0;
	volatile int count = 0;
	strcpy(buf, "0x");
	count = count + 2;

	while (bufpos < 8)
	{
		num_convert_buf[bufpos++] = hex_table[(num & 0x0f)];
		num = (num >> 4);
	}

	while (--bufpos >= 0)
	{
		buf[count++] = num_convert_buf[bufpos];
	}
	buf[count++] = '\0';

	return count;
}

uint32_t str_to_uint32(char* buf)
{
	volatile uint32_t num = 0;
	volatile uint32_t base = 1;
	volatile char* ptr = buf;
	while (*(++ptr) != '\0')
		;

	while (ptr-- > buf)
	{
		num += (((*ptr) - '0') * base);
		base *= 10;
	}
	return num;
}

uint32_t str_to_uint32_len(const char* buf, uint32_t len)
{
	volatile uint32_t num = 0;
	volatile uint32_t base = 1;
	volatile const char* ptr = buf;
	volatile const char* buf_end = buf + len;
	while ((*(++ptr) != '\0') && (ptr < buf_end))
		;

	while (ptr-- > buf)
	{
		num += (((*ptr) - '0') * base);
		base *= 10;
	}
	return num;
}

