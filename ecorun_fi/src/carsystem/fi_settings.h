/*
 * fi_settings.h
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#ifndef CARSYSTEM_FI_SETTINGS_H_
#define CARSYSTEM_FI_SETTINGS_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

// injection
#define DEFAULT_INJECT_TIME 30
#define DEFAULT_INJECT_IDLE_TIME 15
#define DEFAULT_INJECT_START_TIME 24
#define DEFAULT_FUEL_CUT_THRESHOLD 4000
#define DEFAULT_FUEL_CUT_INTERVAL_THRESHOLD 1080000 /* 2880000000 / 4000 */

extern volatile uint32_t inject_time_100us;  // 3.0ms
extern volatile uint32_t inject_time_idle_100us; // 2.1ms
extern volatile uint32_t inject_time_start_100us; // 2.1ms
extern volatile uint32_t idle_rev_th;
extern volatile uint32_t fuel_cut_threshold;
extern volatile uint32_t fuel_cut_interval_threshold;

typedef struct fi_setting_data_t
{
	volatile uint8_t inject_time_map[16][16];
} fi_setting_data;

void fi_set_default(void);

uint32_t get_inject_time_from_map(uint32_t th, uint32_t rev);
void set_inject_time_to_map(uint32_t th, uint32_t rev, uint32_t time);

fi_setting_data* get_fi_setting_data(void);

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_FI_SETTINGS_H_ */
