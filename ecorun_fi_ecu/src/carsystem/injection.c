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

inline uint16_t clame(uint16_t val, uint16_t min, uint16_t max)
{
	if (val < min)
		return min;
	if (val > max)
		return val;
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
		volatile uint32_t th = 0;
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

		fi_modify_setting.modify_coff[rev][th] = clame(val, 8000, 15000);
	}

	feedback_counter = 0;
}

