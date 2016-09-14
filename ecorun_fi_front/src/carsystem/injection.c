/*
 * injection.c
 *
 *  Created on: 2015/03/28
 *      Author: Yoshio
 */

#include "injection.h"

volatile fi_engine_state_data fi_engine_state;
volatile fi_basic_setting_data fi_basic_setting;
volatile fi_modify_setting_data fi_modify_setting;
volatile fi_feedback_setting_data fi_feedback_setting;
volatile fi_switch_state_data fi_switch_state;
volatile fi_starting_setting_data fi_starting_setting;
volatile fi_intake_temperature_correction_data fi_intake_temperature_correction;
volatile fi_oil_temperature_correction_data fi_oil_temperature_correction;

const uint32_t throttle_bounds[] =
{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

const uint32_t throttle_bounds_delta = 10;

const uint32_t rev_bounds[] =
{ 0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000 };

const uint32_t rev_bounds_delta = 500;

const uint32_t temperature_bounds[] =
{ 0, 20, 40, 60, 80, 100, 120 };

const uint32_t temperature_bounds_delta = 20;

volatile uint32_t inject_time_100us = DEFAULT_INJECT_TIME;  // 5ms
volatile uint32_t inject_time_idle_100us = DEFAULT_INJECT_IDLE_TIME; // 3.5ms
volatile uint32_t inject_time_start_100us = DEFAULT_INJECT_START_TIME; // 4ms
volatile uint32_t idle_rev_th = 700;
volatile uint32_t fuel_cut_threshold = DEFAULT_FUEL_CUT_THRESHOLD;
volatile uint32_t fuel_cut_interval_threshold =
DEFAULT_FUEL_CUT_INTERVAL_THRESHOLD;

void fi_set_default(void)
{
	volatile uint32_t rev, th;
	for (rev = 0; rev < NUM_REV_POINTS; rev++)
	{
		for (th = 0; th < NUM_TH_POINTS; th++)
		{
			volatile uint32_t time = 0;
			switch (th)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			default:
				time = th * 20;
				break;
			}

			fi_basic_setting.basic_inject_time_map[rev][th] = time;
		}
	}
}

uint32_t compute_base_inject_time(uint32_t th, uint32_t rev)
{
	volatile uint32_t num, thIndex, th2, revIndex;
	th2 = th >> 2;
	if (th2 < 150)
	{
		th2 = 150;
	}
	num = th2 - 150;
	thIndex = (num >> 6) & 15;
	revIndex = (rev >> 9) & 15;
	return fi_basic_setting.basic_inject_time_map[revIndex][thIndex];
}

void set_inject_time_to_map(uint32_t th, uint32_t rev, uint32_t time)
{
	volatile uint32_t num, thIndex, th2, revIndex;
	th2 = th;
	if (th < 150)
	{
		th2 = 150;
	}
	num = th2 - 150;
	thIndex = (num >> 6) & 15;
	revIndex = (rev >> 9) & 15;
	fi_basic_setting.basic_inject_time_map[revIndex][thIndex] = time;
}

