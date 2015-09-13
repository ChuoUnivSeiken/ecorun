/*
 * fi_settings.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include "fi_settings.h"

fi_setting_data fi_settings;
fi_modify_setting_data fi_modify_setting;
fi_feedback_settings_data fi_feedback_settings;

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
			fi_settings.basic_inject_time_map[rev][th] = 0;
			fi_modify_setting.modify_coff[rev][th] = 10000;
		}
	}

	fi_feedback_settings.delta = 0;
	fi_feedback_settings.enable_feedback = 1;
	fi_feedback_settings.interval = 1;
	fi_feedback_settings.num_step = 1;
}

uint32_t get_inject_time_from_map(uint32_t th, uint32_t rev)
{
	volatile uint32_t thIndex, revIndex;
	thIndex = (th >> 7) & 0x7;
	revIndex = 0;
	if (fi_feedback_settings.enable_feedback)
	{
		return fi_settings.basic_inject_time_map[revIndex][thIndex]
				* fi_modify_setting.modify_coff[revIndex][thIndex] / 10000;
	}
	return fi_settings.basic_inject_time_map[revIndex][thIndex];
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
	fi_settings.basic_inject_time_map[revIndex][thIndex] = time;
}

