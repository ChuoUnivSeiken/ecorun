/*
 * timer_handler.c
 *
 *  Created on: 2014/06/22
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "timer.h"
#include <stdlib.h>

typedef struct timer_event_handler
{
	void (*func)(uint8_t timer, uint8_t num);
	struct timer_event_handler* next;
} timer_event_handler;

timer_event_handler timer_events_buf[TIMER_MAX_EVENT];
uint32_t timer_event_count = 0;

timer_event_handler* timer32_events[2];

uint32_t timer32_add_event(uint32_t timer, timer_event_func func)
{
	timer_event_handler* new_event = &timer_events_buf[timer_event_count];
	new_event->func = func;
	new_event->next = timer32_events[timer];
	timer32_events[timer] = new_event;
	return timer_event_count++;
}

void timer32_notify_event(uint8_t timer, uint8_t num)
{
	volatile timer_event_handler* ptr = timer32_events[timer];
	while (ptr != NULL)
	{
		ptr->func(timer, num);
		ptr = ptr->next;
	}
}

void TIMER32_0_IRQHandler(void)
{
	if (LPC_TMR32B0->IR & _BV(0))
	{
		timer32_notify_event(0, 0);
		LPC_TMR32B0->IR = _BV(0);
	}
	else if (LPC_TMR32B0->IR & _BV(1))
	{
		timer32_notify_event(0, 1);
		LPC_TMR32B0->IR = _BV(1);
	}
	else if (LPC_TMR32B0->IR & _BV(2))
	{
		timer32_notify_event(0, 2);
		LPC_TMR32B0->IR = _BV(2);
	}
	else if (LPC_TMR32B0->IR & _BV(3))
	{
		timer32_notify_event(0, 3);
		LPC_TMR32B0->IR = _BV(3);
	}
}

void TIMER32_1_IRQHandler(void)
{
	if (LPC_TMR32B1->IR & _BV(0))
	{
		timer32_notify_event(1, 0);
		LPC_TMR32B1->IR = _BV(0);
	}
	else if (LPC_TMR32B1->IR & _BV(1))
	{
		timer32_notify_event(1, 1);
		LPC_TMR32B1->IR = _BV(1);
	}
	else if (LPC_TMR32B1->IR & _BV(2))
	{
		timer32_notify_event(1, 2);
		LPC_TMR32B1->IR = _BV(2);
	}
	else if (LPC_TMR32B1->IR & _BV(3))
	{
		timer32_notify_event(1, 3);
		LPC_TMR32B1->IR = _BV(3);
	}
}

timer_event_handler* timer16_events[2];

uint32_t timer16_add_event(uint32_t timer, timer_event_func func)
{
	timer_event_handler* new_event = &timer_events_buf[timer_event_count];
	new_event->func = func;
	new_event->next = timer16_events[timer];
	timer16_events[timer] = new_event;
	return timer_event_count++;
}

void timer16_notify_event(uint8_t timer, uint8_t num)
{
	volatile timer_event_handler* ptr = timer16_events[timer];
	while (ptr != NULL)
	{
		ptr->func(0, num);
		ptr = ptr->next;
	}
}

void TIMER16_0_IRQHandler(void)
{
	if (LPC_TMR16B0->IR & _BV(0))
	{
		timer16_notify_event(0, 0);
		LPC_TMR16B0->IR = _BV(0);
	}
	else if (LPC_TMR16B0->IR & _BV(1))
	{
		timer16_notify_event(0, 1);
		LPC_TMR16B0->IR = _BV(1);
	}
	else if (LPC_TMR16B0->IR & _BV(2))
	{
		timer16_notify_event(0, 2);
		LPC_TMR16B0->IR = _BV(2);
	}
	else if (LPC_TMR16B0->IR & _BV(3))
	{
		timer16_notify_event(0, 3);
		LPC_TMR16B0->IR = _BV(3);
	}
}

void TIMER16_1_IRQHandler(void)
{
	if (LPC_TMR16B1->IR & _BV(0))
	{
		timer16_notify_event(1, 0);
		LPC_TMR16B1->IR = _BV(0);
	}
	else if (LPC_TMR16B1->IR & _BV(1))
	{
		timer16_notify_event(1, 1);
		LPC_TMR16B1->IR = _BV(1);
	}
	else if (LPC_TMR16B1->IR & _BV(2))
	{
		timer16_notify_event(1, 2);
		LPC_TMR16B1->IR = _BV(2);
	}
	else if (LPC_TMR16B1->IR & _BV(3))
	{
		timer16_notify_event(1, 3);
		LPC_TMR16B1->IR = _BV(3);
	}
}

void SysTick_Handler(void)
{
}

