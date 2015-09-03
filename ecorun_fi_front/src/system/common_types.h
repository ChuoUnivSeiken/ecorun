/*
 * common_types.h
 *
 *  Created on: 2015/04/07
 *      Author: Yoshio
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t* string;
typedef const uint8_t* const_string;

typedef uint8_t* buffer;
typedef const uint8_t* const_buffer;

#define _BV(n) (((uint32_t)1) << n)

#endif /* COMMON_TYPES_H_ */
