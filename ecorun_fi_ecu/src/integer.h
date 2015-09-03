/*
 * integer.h
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#ifndef INTEGER_H_
#define INTEGER_H_

#include "type.h"

uint32_t int32_to_str(int32_t num, char* buf);
uint32_t uint32_to_str(uint32_t num, char* buf);
uint32_t uint32_to_hex_str(uint32_t num, char* buf);

uint32_t str_to_uint32(char* buf);
uint32_t str_to_uint32_len(const char* buf, uint32_t len);

#endif /* INTEGER_H_ */
