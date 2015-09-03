/*
 * eeprom_util.h
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#ifndef UTIL_EEPROM_UTIL_H_
#define UTIL_EEPROM_UTIL_H_

#if defined(__cplusplus)
extern "C"
{
#endif

typedef void (*eeprom_print_func)(char ch);
void eeprom_dump(eeprom_print_func print_func);

#if defined(__cplusplus)
}
#endif

#endif /* UTIL_EEPROM_UTIL_H_ */
