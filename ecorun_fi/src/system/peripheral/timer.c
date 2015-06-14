/*
 * timer.c
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#include "../cmsis/LPC13Uxx.h"
#include "timer.h"

void timer32_enable(uint32_t timer)
{
	if (timer == 0)
	{
		LPC_CT32B0->TCR = 1;
	}
	else
	{
		LPC_CT32B1->TCR = 1;
	}
}
void timer32_disable(uint32_t timer)
{
	if (timer == 0)
	{
		LPC_CT32B0->TCR = 0;
	}
	else
	{
		LPC_CT32B1->TCR = 0;
	}
}
void timer32_reset(uint32_t timer)
{
	uint32_t regVal;

	if (timer == 0)
	{
		regVal = LPC_CT32B0->TCR;
	}
	else if (timer == 1)
	{
		regVal = LPC_CT32B1->TCR;
	}
	regVal |= 0x02;
	if (timer == 0)
	{
		LPC_CT32B0->TCR = regVal;
	}
	else if (timer == 1)
	{
		LPC_CT32B1->TCR = regVal;
	}
}
void timer32_init(uint32_t timer, uint32_t interval)
{
	if (timer == 0)
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 9);
		LPC_CT32B0->PR = 0;
		LPC_CT32B0->MR0 = interval;
		LPC_CT32B0->MR1 = interval;
		LPC_CT32B0->MR2 = interval;
		LPC_CT32B0->MR3 = interval;

		LPC_CT32B0->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);
		//LPC_CT32B0->PWMC = _BV(0);
		//LPC_CT32B0->EMR = (1 << 4) | 1;

#if ENABLE_INTERRUPT
		NVIC_EnableIRQ(CT32B0_IRQn);
#endif
	}
	else
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 10);
		LPC_CT32B1->PR = 0;
		LPC_CT32B1->MR0 = interval;
		LPC_CT32B1->MR1 = interval;
		LPC_CT32B1->MR2 = interval;
		LPC_CT32B1->MR3 = interval;

		LPC_CT32B1->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);
		//LPC_CT32B1->MCR = _BV(9) | _BV(10);
		LPC_CT32B1->PWMC = _BV(0) | _BV(1);
		LPC_CT32B1->EMR = (1 << 6) | (1 << 4) | 0x3;

#if ENABLE_INTERRUPT
		NVIC_EnableIRQ(CT32B1_IRQn);
#endif
	}
}

void timer32_set_match(uint32_t timer, uint32_t num, uint32_t value)
{
	if (timer)
	{
		switch (num)
		{
		case 0:
			LPC_CT32B1->MR0 = value;
			break;
		case 1:
			LPC_CT32B1->MR1 = value;
			break;
		case 2:
			LPC_CT32B1->MR2 = value;
			break;
		case 3:
			LPC_CT32B1->MR3 = value;
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
			LPC_CT32B0->MR0 = value;
			break;
		case 1:
			LPC_CT32B0->MR1 = value;
			break;
		case 2:
			LPC_CT32B0->MR2 = value;
			break;
		case 3:
			LPC_CT32B0->MR3 = value;
			break;
		default:
			break;
		}
	}
}

void timer32_set_pwm(uint32_t timer, uint32_t period)
{
	if (timer == 1)
	{
		NVIC_DisableIRQ(CT32B1_IRQn);
		timer32_disable(timer);

		/* Setup the external match register (clear on match) */
		LPC_CT32B1->EMR = _BV(0) | _BV(1) | _BV(2) | _BV(3) | (0x01 << 4) | (0x01 << 6) | (0x01 << 8) | (0x01 << 10);

		/* Set MAT0..3 to PWM mode via the PWM Control register */
		LPC_CT32B1->PWMC = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

		/* MAT3 controls period, set MAT1..3 to 50% duty cycle to start */
		LPC_CT32B1->PR = 0;
		timer32_set_match(timer, 0, period);
		timer32_set_match(timer, 1, period);
		timer32_set_match(timer, 2, period);
		timer32_set_match(timer, 3, period);

		/* Reset on MR3 */
		LPC_CT32B1->MCR = _BV(0) | _BV(1) | _BV(3) | _BV(6) | _BV(9);

		NVIC_EnableIRQ(CT32B1_IRQn);
	}
	else
	{
		NVIC_DisableIRQ(CT32B0_IRQn);
		timer32_disable(timer);

		/* Setup the external match register (clear on match) */
		LPC_CT32B1->EMR = (1 << 10) | (1 << 8) | (1 << 6) | (1 << 4) | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

		/* Set MAT0..3 to PWM mode via the PWM Control register */
		LPC_CT32B1->PWMC = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

		/* MAT3 controls period, set MAT1..3 to 50% duty cycle to start */
		timer32_set_match(timer, 0, period / 2);
		timer32_set_match(timer, 1, period / 2);
		timer32_set_match(timer, 2, period / 2);
		timer32_set_match(timer, 3, period);

		/* Reset on MR3 */
		LPC_CT32B1->MCR = _BV(0) | _BV(3) | _BV(6) | _BV(9) | _BV(10);

		NVIC_EnableIRQ(CT32B0_IRQn);
	}
}

/* TIMER_CLOCKFREQ is the clock rate into the timer prescaler */
#define TIMER_CLOCKFREQ SystemCoreClock

/* MHZ_PRESCALE is a value to set the prescaler to in order to
 clock the timer at 1 MHz. Clock needs to be a multiple of 1 MHz or
 this will not work. */
#define MHZ_PRESCALE    (TIMER_CLOCKFREQ/1000000)

/* TIME_INTERVALmS is a value to load the timer match register with
 to get a 1 mS delay */
#define TIME_INTERVALmS	1000

void timer16_init(uint32_t timer, uint32_t prescale, uint32_t interval)
{
	if (timer == 0)
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7);
		LPC_CT16B0->PR = prescale;
		LPC_CT16B0->MR0 = interval;

		LPC_CT16B0->MCR = 3;

#if ENABLE_INTERRUPT
		NVIC_EnableIRQ(CT16B0_IRQn);
#endif
	}
	else
	{
		LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 8);
		LPC_CT16B1->PR = prescale;
		LPC_CT16B1->MR0 = interval;

		LPC_CT16B1->MCR = 3;

#if ENABLE_INTERRUPT
		NVIC_EnableIRQ(CT16B1_IRQn);
#endif
	}
}
void timer16_set_pwm(uint32_t timer, uint32_t interval)
{

}
void timer16_set_match(uint32_t timer, uint32_t num, uint32_t value)
{

}
void timer16_enable(uint32_t timer)
{
	if (timer == 0)
	{
		LPC_CT16B0->TCR = 1;
	}
	else
	{
		LPC_CT16B1->TCR = 1;
	}
}
void timer16_disable(uint32_t timer)
{
	if (timer == 0)
	{
		LPC_CT16B0->TCR = 0;
	}
	else
	{
		LPC_CT16B1->TCR = 0;
	}
}
void timer16_reset(uint32_t timer)
{
	uint32_t regVal;

	if (timer == 0)
	{
		regVal = LPC_CT16B0->TCR;
	}
	else if (timer == 1)
	{
		regVal = LPC_CT16B1->TCR;
	}
	regVal |= 0x02;
	if (timer == 0)
	{
		LPC_CT16B0->TCR = regVal;
	}
	else if (timer == 1)
	{
		LPC_CT16B1->TCR = regVal;
	}
}

