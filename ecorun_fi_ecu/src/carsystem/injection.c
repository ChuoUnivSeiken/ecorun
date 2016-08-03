/*
 * injection.c
 *
 *  Created on: 2015/03/28
 *      Author: Yoshio
 */

#include "injection.h"

volatile engine_data eg_data;
volatile uint32_t starter_motor_on = 0;

static volatile uint32_t feedback_counter = 0;

static uint16_t clamp(uint16_t val, uint16_t min, uint16_t max)
{
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

void fi_feedback(void)
{
	if (feedback_counter++ < fi_feedback_settings.interval)
	{
		return;
	}

	if (fi_feedback_settings.enable_feedback)
	{
		volatile uint32_t th = calc_th_percentage(eg_data.th) / 10;
		volatile uint32_t rev = 0;
		volatile uint16_t val = fi_modify_setting.modify_coff[rev][th];

		if (eg_data.is_af_rich)
		{
			val -= fi_feedback_settings.delta;
		}
		else
		{
			val += fi_feedback_settings.delta;
		}

		fi_modify_setting.modify_coff[rev][th] = clamp(val, 9000, 11000);
	}

	feedback_counter = 0;
}

fi_setting_data fi_settings;
fi_modify_setting_data fi_modify_setting;
fi_feedback_settings_data fi_feedback_settings;
fi_switch_info_data fi_switch_info;

void fi_set_default(void)
{
	memset((char*)&eg_data, 0, sizeof(eg_data));

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

uint32_t get_inject_time_from_map(void)
{
	if (fi_switch_info.starter_motor_on)
	{
		return 100;
	}

	volatile uint32_t th = eg_data.th;
	volatile uint32_t rev = eg_data.rev;
	volatile uint32_t th_percentage = calc_th_percentage(th);
	volatile uint32_t th_ix1 = calc_th_index(th);
	volatile uint32_t th_ix2 = clamp(th_ix1 + 1, 0, 10);
	volatile uint32_t rev_ix = clamp(rev, 0, 6000) / 12;

	volatile uint32_t a = fi_settings.basic_inject_time_map[rev_ix][th_ix1];
	volatile uint32_t b = fi_settings.basic_inject_time_map[rev_ix][th_ix2];

	volatile uint32_t t = th_percentage - th_bound[th_ix1];

	volatile uint32_t inject_time = a + (b - a) * t / 10;

	if (fi_feedback_settings.enable_feedback)
	{
		return inject_time * fi_modify_setting.modify_coff[rev_ix][th_ix1]
				/ 10000;
	}
	return inject_time;
}

uint32_t calc_th_percentage(uint32_t th_val)
{
	return (th_val * 100) >> 10;
}

uint32_t calc_th_index(uint32_t th_val)
{
	return calc_th_percentage(th_val) / 10;
}

