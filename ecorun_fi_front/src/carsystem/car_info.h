/*
 * car_info.h
 *
 *  Created on: 2015/03/28
 *      Author: Yoshio
 */

#ifndef CARSYSTEM_CAR_INFO_H_
#define CARSYSTEM_CAR_INFO_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct car_data_t
{
	uint32_t vattery_voltage;
	uint32_t wheel_count;
	uint32_t wheel_rotation_period;
} car_data;

extern volatile car_data cr_data;

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_CAR_INFO_H_ */
