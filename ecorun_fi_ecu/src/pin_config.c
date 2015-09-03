/*
 * pin_config.c
 *
 *  Created on: 2014/07/01
 *      Author: Yoshio
 */

#include "cmsis/LPC11xx.h"
#include "pin_config.h"
#include "type.h"

void conf_set_func(uint32_t* pin, uint8_t func) {
	*pin &= ~0x07;
	*pin |= func;
}

void conf_set_admode(uint32_t* pin, uint8_t admode) {
	if (admode == PIN_ADMODE_ANALOG) {
		*pin &= ~_BV(7);
	} else if (admode == PIN_ADMODE_DIGITAL) {
		*pin |= _BV(7);
	}
}
