/*
 * countdown_timer.c
 *
 *  Created on: 2015/10/08
 *      Author: Yoshio
 */

#include "countdown_timer.h"
#include "system/peripheral/timer.h"
#include "system/cmsis/LPC13Uxx.h"

volatile uint32_t countdown_timer_val = 0;

void timer16_1_handler(uint8_t timer, uint8_t num)
{
	if (num == 0)
	{
		if (countdown_timer_val > 0)
		{
			countdown_timer_val--;
		}
		else
		{
			timer16_disable(1);
		}
	}
}

void countdown_timer_init(void)
{
	timer16_init(1, 1000, SystemCoreClock / 1000 / 1000);
	timer16_add_event(1, timer16_1_handler);
}

void delay_ms(uint32_t ms)
{
	countdown_timer_val = ms;

	timer16_reset(1);
	timer16_enable(1);

	while (countdown_timer_val > 0)
		;
}

void start_countdown(uint32_t ms)
{
	countdown_timer_val = ms;

	timer16_reset(1);
	timer16_enable(1);
}

