/*
 * base64.h
 *
 *  Created on: 2014/08/27
 *      Author: Yoshio
 */

#ifndef BASE64_H_
#define BASE64_H_

#include "../system/common_types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

void usart_write_base64(const_buffer data, uint32_t data_size);
uint32_t encode_base64(const_buffer data, uint32_t data_size, string dest);
uint32_t decode_base64_s(const_string base64, uint8_t* dest, uint32_t dest_size);
uint32_t decode_base64(const_string base64, string dest);

#if defined(__cplusplus)
}
#endif

#endif /* BASE64_H_ */
