/*
 * usart_util.h
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#ifndef UTIL_USART_UTIL_H_
#define UTIL_USART_UTIL_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

// send interger with '\0'
void usart_writeln_int32(int32_t value);
void usart_writeln_uint32(uint32_t value);
void usart_writeln_uint32_hex(uint32_t value);

void usart_write_int32(int32_t value);
void usart_write_uint32(uint32_t value);
void usart_write_uint8_hex(uint32_t value);
void usart_write_uint32_hex(uint32_t value);

#if defined(__cplusplus)
}
#endif

#endif /* UTIL_USART_UTIL_H_ */
