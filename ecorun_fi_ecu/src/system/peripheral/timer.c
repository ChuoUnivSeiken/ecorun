/*
 * timer.c
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "timer.h"

void timer32_enable(uint8_t timer)
{
	if (timer == 1)
	{
		LPC_TMR32B1->TCR = 1;
	}
	else
	{
		LPC_TMR32B0->TCR = 1;
	}
}

void timer32_disable(uint8_t timer)
{
	if (timer == 1)
	{
		LPC_TMR32B1->TCR = 0;
	}
	else
	{
		LPC_TMR32B0->TCR = 0;
	}
}

void timer32_reset(uint8_t timer)
{
	if (timer == 1)
	{
		uint32_t regVal;
		regVal = LPC_TMR32B1->TCR;
		regVal |= 0x02;
		LPC_TMR32B1->TCR = regVal;
	}
	else
	{
		uint32_t regVal;
		regVal = LPC_TMR32B0->TCR;
		regVal |= 0x02;
		LPC_TMR32B0->TCR = regVal;
	}
}

void timer32_init(uint8_t timer, uint32_t interval)
{
	if (timer == 1)
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 10);
		LPC_TMR32B1->PR = 0;
		LPC_TMR32B1->MR0 = interval * 200 / 1023;
		LPC_TMR32B1->MR1 = interval * 200 / 1023;
		LPC_TMR32B1->MR2 = interval / 8;
		LPC_TMR32B1->MR3 = interval;

		//LPC_TMR32B1->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);
		LPC_TMR32B1->MCR = _BV(9) | _BV(10);
		LPC_TMR32B1->PWMC = _BV(0) | _BV(1);
		LPC_TMR32B1->EMR = (1 << 6) | (1 << 4) | 0x3;
		NVIC_EnableIRQ(TIMER_32_1_IRQn);
	}
	else
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 9);
		LPC_TMR32B0->PR = 0;
		LPC_TMR32B0->MR0 = interval;
		LPC_TMR32B0->MR1 = interval;
		LPC_TMR32B0->MR2 = interval;
		LPC_TMR32B0->MR3 = interval;

		LPC_TMR32B0->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);
		//LPC_TMR32B0->PWMC = _BV(0);
		//LPC_TMR32B0->EMR = (1 << 4) | 1;
		NVIC_EnableIRQ(TIMER_32_0_IRQn);
	}
}

void timer16_enable(uint8_t timer)
{
	if (timer == 1)
	{
		LPC_TMR16B1->TCR = 1;
	}
	else
	{
		LPC_TMR16B0->TCR = 1;
	}
}

void timer16_disable(uint8_t timer)
{
	if (timer == 1)
	{
		LPC_TMR16B1->TCR = 0;
	}
	else
	{
		LPC_TMR16B0->TCR = 0;
	}
}

void timer16_reset(uint8_t timer)
{
	if (timer == 1)
	{
		uint32_t regVal;
		regVal = LPC_TMR16B1->TCR;
		regVal |= 0x02;
		LPC_TMR16B1->TCR = regVal;
	}
	else
	{
		uint32_t regVal;
		regVal = LPC_TMR16B0->TCR;
		regVal |= 0x02;
		LPC_TMR16B0->TCR = regVal;
	}
}

void timer16_init(uint8_t timer, uint32_t interval)
{
	if (timer == 1)
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 8);
		LPC_TMR16B1->PR = 0;
		LPC_TMR16B1->MR0 = interval / 2;
		LPC_TMR16B1->MR1 = interval / 2;
		LPC_TMR16B1->MR2 = interval / 2;
		LPC_TMR16B1->MR3 = interval;

		LPC_TMR16B1->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);

		/* Enable the TIMER1 Interrupt */
		NVIC_EnableIRQ(TIMER_16_1_IRQn);
	}
	else
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7);
		LPC_TMR16B0->PR = 0;
		LPC_TMR16B0->MR0 = interval / 2;
		LPC_TMR16B0->MR1 = interval / 2;
		LPC_TMR16B0->MR2 = interval / 2;
		LPC_TMR16B0->MR3 = interval;

		LPC_TMR16B0->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);

		/* Enable the TIMER0 Interrupt */
		NVIC_EnableIRQ(TIMER_16_0_IRQn);
	}
}

void timer16_set_pwm(uint32_t timer, uint32_t period)
{
	if (timer == 1)
	{
		NVIC_DisableIRQ(TIMER_16_1_IRQn);
		timer16_disable(timer);

		/* Setup the external match register (clear on match) */
		LPC_TMR16B1->EMR = _BV(0) | _BV(1) | _BV(2) | _BV(3) | (0x01 << 4)
				| (0x01 << 6) | (0x01 << 8) | (0x01 << 10);

		/* Set MAT0..3 to PWM mode via the PWM Control register */
		LPC_TMR16B1->PWMC = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

		/* MAT3 controls period, set MAT1..3 to 50% duty cycle to start */
		LPC_TMR16B1->PR = 0;
		timer16_set_match(timer, 0, period / 2);
		timer16_set_match(timer, 1, period / 2);
		timer16_set_match(timer, 2, period / 2);
		timer16_set_match(timer, 3, period);

		/* Reset on MR3 */
		LPC_TMR16B1->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);

		NVIC_EnableIRQ(TIMER_16_1_IRQn);
	}
	else
	{
		NVIC_DisableIRQ(TIMER_16_0_IRQn);
		timer16_disable(timer);

		/* Setup the external match register (clear on match) */
		LPC_TMR16B0->EMR = (1 << 10) | (1 << 8) | (1 << 6) | (1 << 4) | (1 << 0)
				| (1 << 1) | (1 << 2) | (1 << 3);

		/* Set MAT0..3 to PWM mode via the PWM Control register */
		LPC_TMR16B0->PWMC = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

		/* MAT3 controls period, set MAT1..3 to 50% duty cycle to start */
		timer16_set_match(timer, 0, period / 2);
		timer16_set_match(timer, 1, period / 2);
		timer16_set_match(timer, 2, period / 2);
		timer16_set_match(timer, 3, period);

		/* Reset on MR3 */
		LPC_TMR16B0->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);

		NVIC_EnableIRQ(TIMER_16_0_IRQn);
	}
}

void timer16_set_match(uint32_t timer, uint32_t num, uint32_t value)
{
	if (timer == 1)
	{
		switch (num)
		{
		case 0:
			LPC_TMR16B1->MR0 = value;
			break;
		case 1:
			LPC_TMR16B1->MR1 = value;
			break;
		case 2:
			LPC_TMR16B1->MR2 = value;
			break;
		case 3:
			LPC_TMR16B1->MR3 = value;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (num)
		{
		case 0:
			LPC_TMR16B0->MR0 = value;
			break;
		case 1:
			LPC_TMR16B0->MR1 = value;
			break;
		case 2:
			LPC_TMR16B0->MR2 = value;
			break;
		case 3:
			LPC_TMR16B0->MR3 = value;
			break;
		default:
			break;
		}
	}
}

