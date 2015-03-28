/*
 * adler32.h
 *
 *  Created on: 2014/08/08
 *      Author: Yoshio
 */

#ifndef ADLER32_H_
#define ADLER32_H_

#include <stdint.h>
#include <stddef.h>

#define MOD_ADLER 65521

#if defined(__cplusplus)
extern "C"
{
#endif

uint32_t adler32(const uint8_t *data, size_t len);

#if defined(__cplusplus)
}
#endif

#endif /* ADLER32_H_ */
