/*
 * injection.h
 *
 *  Created on: 2015/03/28
 *      Author: Yoshio
 */

#ifndef CARSYSTEM_INJECTION_H_
#define CARSYSTEM_INJECTION_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

typedef volatile struct engine_data_t
{
	uint32_t rev;
	uint32_t is_fuel_cut;
	uint32_t is_af_rich;
	uint32_t th;
	uint32_t oil_temp;
	uint32_t intake_temp;
	uint32_t vacuum;
	uint32_t current_total_injected_time;
	uint32_t current_inject_started_count;
	uint32_t current_inject_ended_count;
} engine_data;

extern volatile engine_data eg_data;
extern volatile uint32_t starter_motor_on;

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_INJECTION_H_ */
