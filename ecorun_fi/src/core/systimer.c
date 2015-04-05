/*
 * systimer.c
 *
 *  Created on: 2015/04/03
 *      Author: Yoshio
 */

#include "../system/cmsis/LPC13Uxx.h"
#include "../system/cmsis/core_cm3.h"

static volatile uint32_t counter = 0;

void systimer_init(uint32_t fraq)
{
	SysTick->LOAD = SystemCoreClock / fraq - 1;
	SysTick->CTRL = 0x07;
}
uint32_t systimer_tick(void)
{
	return SysTick->VAL;
}
uint32_t systimer_freq(void)
{
	return SystemCoreClock;
}

void SysTick_Handler(void)
{
	counter++;
}

