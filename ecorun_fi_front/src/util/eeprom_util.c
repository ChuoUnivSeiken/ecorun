/*
 * eeprom_util.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include "eeprom_util.h"
#include "../system/peripheral/eeprom.h"
#include <string.h>

void eeprom_dump(eeprom_print_func print_func)
{
	uint32_t i, j;
	uint8_t z;
	char valBuff[12];
	char lineBuff[80];
	for (i = 0; i < EEPROM_SIZE; i += 16)
	{
		// TODO: implement
		//sprintf(lineBuff, "0x%03X ", (unsigned int) i);
		for (j = 0; j < 16; j++)
		{
			eeprom_read((uint8_t*) i + j, (uint8_t*) &z, 1);
			//sprintf(valBuff, "%02X ", (unsigned int) z);
			strcat(lineBuff, valBuff);
		}
		strcat(lineBuff, "\r\n");
		//printf((char*) lineBuff);
	}
}
