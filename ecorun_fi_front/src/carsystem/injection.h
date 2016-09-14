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

#define AF_LEAN	(0)
#define AF_RICH	(1)
#define AF_NA	(2)

/* エンジンセンシング情報 */
typedef struct
{
	volatile uint32_t rev; // [rpm]
	volatile uint32_t is_fuel_cut;
	volatile uint32_t af;
	volatile uint32_t th; // [%]
	volatile uint32_t oil_temp; // [K]
	volatile uint32_t intake_temp; // [K]
	volatile uint32_t vacuum;
	volatile uint32_t checksum;
} fi_engine_state_data;

#define NUM_TH_POINTS 11
extern const uint32_t throttle_bounds[NUM_TH_POINTS];
extern const uint32_t throttle_bounds_delta;

#define NUM_REV_POINTS 13
extern const uint32_t rev_bounds[NUM_REV_POINTS];
extern const uint32_t rev_bounds_delta;

#define NUM_TEMPERATURE_POINTS 7
extern const uint32_t temperature_bounds[NUM_TEMPERATURE_POINTS];
extern const uint32_t temperature_bounds_delta;

/* 基本噴射時間 */
typedef struct
{
	volatile uint8_t basic_inject_time_map[NUM_REV_POINTS][NUM_TH_POINTS];
	volatile uint32_t checksum;
} fi_basic_setting_data;

/* 始動時設定 */
typedef struct
{
	volatile uint8_t base_temperature;	// 基本温度 [K]
	volatile uint8_t inject_time[NUM_TEMPERATURE_POINTS];	// 噴射時間-温度
	volatile uint32_t checksum;
} fi_starting_setting_data;

/* 吸気温補正 */
typedef struct
{
	volatile uint8_t base_temperature;	// 基本温度 [K]
	volatile uint16_t coefficient[NUM_TEMPERATURE_POINTS];	// 補正係数-温度
	volatile uint32_t checksum;
} fi_intake_temperature_correction_data;

/* 水温補正 */
typedef struct
{
	volatile uint8_t base_temperature;	// 基本温度 [K]
	volatile uint16_t coefficient[NUM_TEMPERATURE_POINTS];	// 補正係数-温度
	volatile uint32_t checksum;
} fi_oil_temperature_correction_data;

/* 基本噴射時間補正値 */
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
} fi_feedback_setting_data;

typedef struct
{
	volatile uint32_t starter_motor_on;
	volatile uint32_t fuel_pump_on;
	volatile uint32_t cdi_on;
} fi_switch_state_data;

/* states */
extern volatile fi_engine_state_data fi_engine_state;
extern volatile fi_switch_state_data fi_switch_state;

/* settings */
extern volatile fi_basic_setting_data fi_basic_setting;
extern volatile fi_modify_setting_data fi_modify_setting;
extern volatile fi_feedback_setting_data fi_feedback_setting;
extern volatile fi_starting_setting_data fi_starting_setting;
extern volatile fi_intake_temperature_correction_data fi_intake_temperature_correction;
extern volatile fi_oil_temperature_correction_data fi_oil_temperature_correction;

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

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_INJECTION_H_ */
