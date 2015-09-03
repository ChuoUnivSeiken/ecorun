/*
 * systimer.c
 *
 *  Created on: 2015/04/03
 *      Author: Yoshio
 */

#include "cmsis/LPC13Uxx.h"
#include "cmsis/core_cm3.h"
#include "systimer.h"

static volatile uint32_t counter = 0;
static volatile uint32_t sub_counter = 0;

void systimer_init(void)
{
	SysTick->LOAD = SystemCoreClock / 32 - 1;
	SysTick->CTRL = 0x07;
}

systime_t systimer_tick(void)
{
	uint32_t reg = SysTick->VAL;
	uint32_t us = (SystemCoreClock * sub_counter / 32 + reg) / (SystemCoreClock / 1000000);
	systime_t tick =
	{ counter, us };
	return tick;
}
uint32_t systimer_freq(void)
{
	return SystemCoreClock;
}

void SysTick_Handler(void)
{
	if (++sub_counter == 32)
	{
		counter++;
		sub_counter = 0;
	}
}

