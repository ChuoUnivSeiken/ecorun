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

#define NUM_TH_POINTS 11
#define NUM_REV_POINTS 13

typedef struct
{
	volatile uint8_t basic_inject_time_map[NUM_REV_POINTS][NUM_TH_POINTS];
	volatile uint8_t padding;
	volatile uint32_t checksum;
} fi_setting_data;

typedef struct
{
	volatile uint16_t modify_coff[NUM_REV_POINTS][NUM_TH_POINTS];
} fi_modify_setting_data;

typedef struct
{
	volatile uint32_t enable_feedback;
	volatile uint32_t interval;
	volatile uint32_t delta;
	volatile uint32_t num_step;
	volatile uint32_t checksum;
} fi_feedback_settings_data;

typedef struct
{
	volatile uint32_t starter_motor_on;
	volatile uint32_t fuel_pump_on;
	volatile uint32_t cdi_on;
} fi_switch_info_data;

void fi_set_default(void);

// th = 0 ~ 1023
uint32_t get_inject_time_from_map(void);
uint32_t calc_th_percentage(uint32_t th_val);
uint32_t calc_th_index(uint32_t th_val);

extern engine_data eg_data;
extern fi_setting_data fi_settings;
extern fi_modify_setting_data fi_modify_setting;
extern fi_feedback_settings_data fi_feedback_settings;
extern fi_switch_info_data fi_switch_info;

void fi_feedback(void);

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_INJECTION_H_ */
