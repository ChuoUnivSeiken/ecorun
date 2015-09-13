/*
 * system_io.h
 *
 *  Created on: 2015/09/04
 *      Author: Yoshio
 */

#ifndef CARSYSTEM_SYSTEM_IO_H_
#define CARSYSTEM_SYSTEM_IO_H_

#include "../type.h"

void set_starter_sw(bool on);
void set_cdi_sw(bool on);
void set_fuel_sw(bool on);

uint32_t get_af(void);

void init_io(void);

#endif /* CARSYSTEM_SYSTEM_IO_H_ */
