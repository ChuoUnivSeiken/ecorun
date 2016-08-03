/*
 * adler32.h
 *
 *  Created on: 2014/08/08
 *      Author: Yoshio
 */

#ifndef ADLER32_H_
#define ADLER32_H_

#include "../type.h"

uint32_t adler32(volatile uint8_t *data, size_t len);

#endif /* ADLER32_H_ */
