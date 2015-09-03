/*
 * fi_settings.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include "fi_settings.h"

fi_setting_data fi_settings;

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
	for (rev = 0; rev < 8; rev++)
	{
		for (th = 0; th < 8; th++)
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
			default:
				time = th * 25;
				break;
			}

			fi_settings.basic_inject_time_map[rev][th] = time;
		}
	}
}

uint32_t get_inject_time_from_map(uint32_t th, uint32_t rev)
{
	volatile uint32_t thIndex, revIndex;
	thIndex = (th >> 7) & 0x7;
	revIndex = 0;
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

fi_setting_data* get_fi_setting_data(void)
{
	return &fi_settings;
}
