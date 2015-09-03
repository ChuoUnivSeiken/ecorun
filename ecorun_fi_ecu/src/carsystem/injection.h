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
	volatile uint32_t rev;
	volatile uint32_t is_fuel_cut;
	volatile uint32_t is_af_rich;
	volatile uint32_t th;
	volatile uint32_t oil_temp;
	volatile uint32_t intake_temp;
	volatile uint32_t vacuum;
	volatile uint32_t current_total_injected_time;
	volatile uint32_t current_inject_started_count;
	volatile uint32_t current_inject_ended_count;
	volatile uint32_t checksum;
} engine_data;

extern volatile engine_data eg_data;
extern volatile uint32_t starter_motor_on;

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_INJECTION_H_ */
