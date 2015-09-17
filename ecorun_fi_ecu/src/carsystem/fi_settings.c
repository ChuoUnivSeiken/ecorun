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
			fi_settings.basic_inject_time_map[rev][th] = 200;
			fi_modify_setting.modify_coff[rev][th] = 10000;
		}
	}

	fi_feedback_settings.delta = 100;
	fi_feedback_settings.enable_feedback = 1;
	fi_feedback_settings.interval = 1;
	fi_feedback_settings.num_step = 1;
}

static volatile uint32_t th_bound[] =
{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

uint32_t get_inject_time_from_map(uint32_t th, uint32_t rev)
{
	volatile uint32_t thIndex, revIndex;
	thIndex = calc_th_index(th);
	revIndex = 0;
	volatile uint32_t th_ix1 = thIndex / 10;
	volatile uint32_t th_ix2 = th_ix1 + 1;

	if (th_ix2 > 10)
		th_ix2 = 10;

	volatile uint32_t a = fi_settings.basic_inject_time_map[revIndex][th_ix1];
	volatile uint32_t b = fi_settings.basic_inject_time_map[revIndex][th_ix2];

	volatile uint32_t t = thIndex - th_bound[th_ix1];

	volatile uint32_t inject_time = a + (b - a) * t / 9;

	revIndex = 0;
	if (fi_feedback_settings.enable_feedback)
	{
		return inject_time * fi_modify_setting.modify_coff[revIndex][th_ix1]
				/ 10000;
	}
	return inject_time;
}

uint32_t calc_th_index(uint32_t th_val)
{
	return (th_val * 100) >> 10;
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

