/*
 * base64.h
 *
 *  Created on: 2014/08/27
 *      Author: Yoshio
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

void usart_write_base64(const uint8_t* data, uint32_t data_size);
uint32_t encode_base64(const uint8_t* data, uint32_t data_size, uint8_t* base64);
uint32_t decode_base64(const uint8_t* base64, uint32_t base64_size,
		uint8_t* data);

#if defined(__cplusplus)
}
#endif

#endif /* BASE64_H_ */
