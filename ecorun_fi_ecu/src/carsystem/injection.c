/*
 * injection.c
 *
 *  Created on: 2015/03/28
 *      Author: Yoshio
 */

#include "injection.h"
#include <string.h>

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

#define NUM_ELEMENTS(arr) (sizeof(arr) / sizeof(arr[0]))

static const uint32_t DEFAULT_STARTING_INJECT_TIME = 40; // 400us
static const uint32_t DEFAULT_INJECT_TIME = 40; // 400us

#define TEMP_CORRECTION_BASE 250

#define COEFFICIENT_ONE 10000

void fi_set_default_settings(void)
{
	memset((char*) &fi_basic_setting, 0, sizeof(fi_basic_setting));
	memset((char*) &fi_modify_setting, 0, sizeof(fi_modify_setting));
	memset((char*) &fi_feedback_setting, 0, sizeof(fi_feedback_setting));
	memset((char*) &fi_switch_state, 0, sizeof(fi_switch_state));

	memset((char*) fi_starting_setting.inject_time, DEFAULT_STARTING_INJECT_TIME,
			sizeof(fi_starting_setting.inject_time));
	fi_starting_setting.base_temperature = TEMP_CORRECTION_BASE;

	memset(fi_intake_temperature_correction.coefficient, COEFFICIENT_ONE,
			NUM_ELEMENTS(fi_intake_temperature_correction.coefficient));
	fi_intake_temperature_correction.base_temperature = TEMP_CORRECTION_BASE;

	memset(fi_oil_temperature_correction.coefficient, COEFFICIENT_ONE,
			NUM_ELEMENTS(fi_oil_temperature_correction.coefficient));
	fi_oil_temperature_correction.base_temperature = TEMP_CORRECTION_BASE;

	volatile uint32_t rev, th;
	for (rev = 0; rev < NUM_REV_POINTS; rev++)
	{
		for (th = 0; th < NUM_TH_POINTS; th++)
		{
			fi_basic_setting.basic_inject_time_map[rev][th] = DEFAULT_INJECT_TIME;
			fi_modify_setting.modify_coff[rev][th] = COEFFICIENT_ONE;
		}
	}

	fi_feedback_setting.delta = 100;
	fi_feedback_setting.enable_feedback = 1;
	fi_feedback_setting.interval = 1;
	fi_feedback_setting.num_step = 1;
}

void fi_set_default(void)
{
	memset((char*) &fi_engine_state, 0, sizeof(fi_engine_state));

	fi_set_default_settings();
}

static uint32_t min(uint32_t val, uint32_t min_val)
{
	if (val < min_val)
	{
		return min_val;
	}
	return val;
}

static uint32_t max(uint32_t val, uint32_t max_val)
{
	if (val > max_val)
	{
		return max_val;
	}
	return val;
}

static uint32_t clamp(uint32_t val, uint32_t min_val, uint32_t max_val)
{
	return max(min(val, max_val), min_val);
}

static uint32_t bilinear_interp(uint32_t z11, uint32_t z12, uint32_t z21,
		uint32_t z22, uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2,
		uint32_t x, uint32_t y)
{
	if ((x2 == x1) || (y2 == y1))
	{
		return z11;
	}

	uint32_t x2x1, y2y1, x2x, y2y, yy1, xx1;
	x2x1 = x2 - x1;
	y2y1 = y2 - y1;
	x2x = x2 - x;
	y2y = y2 - y;
	yy1 = y - y1;
	xx1 = x - x1;
	return (z11 * x2x * y2y + z21 * xx1 * y2y + z12 * x2x * yy1
			+ z22 * xx1 * yy1) / (x2x1 * y2y1);
}

static uint32_t linear_interp(uint32_t y1, uint32_t y2, uint32_t x1,
		uint32_t x2, uint32_t x)
{
	if (x2 == x1)
	{
		return y1;
	}

	return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

static uint16_t compute_temperature_value_uint16(uint32_t temp,
		uint32_t base_temp, uint16_t* coefficient)
{
	uint32_t min_temp = base_temp + temperature_bounds[0];
	uint32_t max_temp = base_temp
			+ temperature_bounds[NUM_TEMPERATURE_POINTS - 1];
	temp = clamp(temp, min_temp, max_temp);

	uint32_t x = temp - base_temp;

	uint32_t idx1 = x / temperature_bounds_delta;
	uint32_t idx2 = min(idx1, NUM_TEMPERATURE_POINTS - 1);

	uint32_t y1 = coefficient[idx1];
	uint32_t y2 = coefficient[idx2];

	uint32_t x1 = temperature_bounds[idx1];
	uint32_t x2 = temperature_bounds[idx2];

	return linear_interp(y1, y2, x1, x2, x);
}

static uint8_t compute_temperature_value_uint8(uint32_t temp,
		uint32_t base_temp, uint8_t* coefficient)
{
	uint32_t min_temp = base_temp + temperature_bounds[0];
	uint32_t max_temp = base_temp
			+ temperature_bounds[NUM_TEMPERATURE_POINTS - 1];
	temp = clamp(temp, min_temp, max_temp);

	uint32_t x = temp - base_temp;

	uint32_t idx1 = x / temperature_bounds_delta;
	uint32_t idx2 = min(idx1, NUM_TEMPERATURE_POINTS - 1);

	uint32_t y1 = coefficient[idx1];
	uint32_t y2 = coefficient[idx2];

	uint32_t x1 = temperature_bounds[idx1];
	uint32_t x2 = temperature_bounds[idx2];

	return linear_interp(y1, y2, x1, x2, x);
}

static uint32_t apply_intake_temperature_correction(uint32_t value)
{
	uint32_t base_temperature = fi_intake_temperature_correction.base_temperature;
	uint16_t* coefficient = fi_intake_temperature_correction.coefficient;
	uint32_t temp = fi_engine_state.intake_temp;

	return value
			* compute_temperature_value_uint16(temp, base_temperature,
					coefficient) / COEFFICIENT_ONE;
}

static uint32_t apply_oil_temperature_correction(uint32_t value)
{
	uint32_t base_temperature = fi_oil_temperature_correction.base_temperature;
	uint16_t* coefficient = fi_oil_temperature_correction.coefficient;
	uint32_t temp = fi_engine_state.oil_temp;

	return value
			* compute_temperature_value_uint16(temp, base_temperature,
					coefficient) / COEFFICIENT_ONE;
}

static uint32_t apply_coefficients(uint32_t value)
{
	value = apply_intake_temperature_correction(value);
	value = apply_oil_temperature_correction(value);

	return value;
}

static uint32_t compute_starting_inject_time(void)
{
	uint32_t base_temperature = fi_starting_setting.base_temperature;
	uint8_t* coefficient = fi_starting_setting.inject_time;
	uint32_t temp = fi_engine_state.oil_temp;

	return compute_temperature_value_uint8(temp, base_temperature, coefficient);
}

uint32_t compute_base_inject_time(void)
{
	if (fi_switch_state.starter_motor_on)
	{
		return compute_starting_inject_time();
	}

	uint32_t th = clamp(fi_engine_state.th, 0, 100);
	uint32_t rev = clamp(fi_engine_state.rev, 0, 6000);

	uint32_t th_ix1 = calc_th_index(th);
	uint32_t th_ix2 = clamp(th_ix1 + 1, 0, 10);

	uint32_t rev_ix1 = calc_rev_index(rev);
	uint32_t rev_ix2 = clamp(rev_ix1 + 1, 0, 12);

	uint32_t y1 = throttle_bounds[th_ix1];
	uint32_t y2 = throttle_bounds[th_ix2];
	uint32_t x1 = rev_bounds[rev_ix1];
	uint32_t x2 = rev_bounds[rev_ix2];

	uint32_t z11 = fi_basic_setting.basic_inject_time_map[rev_ix1][th_ix1];
	uint32_t z12 = fi_basic_setting.basic_inject_time_map[rev_ix1][th_ix2];
	uint32_t z21 = fi_basic_setting.basic_inject_time_map[rev_ix2][th_ix1];
	uint32_t z22 = fi_basic_setting.basic_inject_time_map[rev_ix2][th_ix2];

	uint32_t base_inject_time = bilinear_interp(z11, z12, z21, z22, x1, x2, y1,
			y2, rev, th);

	uint32_t inject_time = apply_coefficients(base_inject_time);

	if (fi_feedback_setting.enable_feedback)
	{
		z11 = fi_modify_setting.modify_coff[rev_ix1][th_ix1];
		z12 = fi_modify_setting.modify_coff[rev_ix1][th_ix2];
		z21 = fi_modify_setting.modify_coff[rev_ix2][th_ix1];
		z22 = fi_modify_setting.modify_coff[rev_ix2][th_ix2];

		uint32_t modify_coeff = bilinear_interp(z11, z12, z21, z22, x1, x2, y1,
				y2, rev, th);

		return inject_time * modify_coeff / COEFFICIENT_ONE;
	}
	return inject_time;
}

void fi_feedback(void)
{
	static volatile uint32_t feedback_counter;

	if (!fi_feedback_setting.enable_feedback)
	{
		return;
	}

	if (feedback_counter++ < fi_feedback_setting.interval)
	{
		return;
	}

	feedback_counter = 0;

	volatile uint32_t th_ix = calc_th_index(fi_engine_state.th);
	volatile uint32_t rev = 0;
	volatile uint16_t val = fi_modify_setting.modify_coff[rev][th_ix];

	if (fi_engine_state.af)
	{
		val -= fi_feedback_setting.delta;
	}
	else
	{
		val += fi_feedback_setting.delta;
	}

	fi_modify_setting.modify_coff[rev][th_ix] = clamp(val, 9000, 11000);
}

uint32_t calc_th_percentage(uint32_t th_val)
{
	return (th_val * 100) >> 10;
}

uint32_t calc_th_index(uint32_t th_val)
{
	return calc_th_percentage(th_val) / 10;
}

uint32_t calc_rev_index(uint32_t rev_val)
{
	return clamp(rev_val, 0, 6000) / 500;
}

