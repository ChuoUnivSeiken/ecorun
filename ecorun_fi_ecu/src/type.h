/*
 * type.h
 *
 *  Created on: 2014/06/22
 *      Author: Yoshio
 */

#ifndef TYPE_H_
#define TYPE_H_

#if defined   (  __GNUC__  )
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#else

/* exact-width signed integer types */
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed __int64 int64_t;

/* exact-width unsigned integer types */
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#endif

#define _BV(n) (((uint32_t)1) << n)

#endif /* TYPE_H_ */
