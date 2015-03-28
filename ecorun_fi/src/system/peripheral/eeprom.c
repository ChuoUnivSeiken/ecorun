/*
 * Copyright (c) 2013, K. Townsend (microBuilder.eu)
 * https://github.com/microbuilder/
 * All rights reserved.
 *
 * Modified by Yoshio Nakamura, Copyright (c) 2015
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

#include "eeprom.h"

#include <string.h>
#include "eeprom.h"
#include "../cmsis/LPC13Uxx.h"

typedef void (*IAP)(unsigned int[], unsigned int[]);
static const IAP iap_entry = (IAP) 0x1FFF1FF1;

RAMFUNC err_t eeprom_write(uint8_t* rom_address, uint8_t* buf, uint32_t count)
{
	unsigned int command[5], result[4];
	/* EEPROM Write : IAP Command Code : 61
	 Param 1 : EEPROM address
	 Param 2 : RAM address of data/buffer to write
	 Param 3 : Number of bytes to be written
	 Param 4 : System clock frequency in kHz (SystemCoreClock/1000)
	 Return Codes : 0 - CMD_SUCCESS
	 4 - SRC_ADDR_NOT_MAPPED
	 5 - DST_ADDR_NOT_MAPPED */
	command[0] = 61;
	command[1] = (uint32_t) rom_address;
	command[2] = (uint32_t) buf;
	command[3] = count;
	command[4] = (uint32_t) (SystemCoreClock / 1000);
	/* Invoke IAP call (interrupts need to be disabled during IAP calls)...*/
	__disable_irq();
	iap_entry(command, result);
	__enable_irq();
	if (0 != result[0])
	{
		return EFAULT;
	}
	return 0;
}

RAMFUNC err_t eeprom_read(uint8_t* rom_address, uint8_t* buf, uint32_t count)
{
	unsigned int command[5], result[4];
	/* EEPROM Read : IAP Command Code : 62
	 Param 1 : EEPROM address
	 Param 2 : RAM address to store data
	 Param 3 : Number of bytes to read
	 Param 4 : System clock frequency in kHz (SystemCoreClock/1000)
	 Return Codes : 0 - CMD_SUCCESS
	 4 - SRC_ADDR_NOT_MAPPED
	 5 - DST_ADDR_NOT_MAPPED */
	command[0] = 62;
	command[1] = (uint32_t) rom_address;
	command[2] = (uint32_t) buf;
	command[3] = count;
	command[4] = (uint32_t) (SystemCoreClock / 1000);
	/* Invoke IAP call (interrupts need to be disabled during IAP calls)...*/
	__disable_irq();
	iap_entry(command, result);
	__enable_irq();
	if (0 != result[0])
	{
		return EFAULT;
	}
	return 0;
}
