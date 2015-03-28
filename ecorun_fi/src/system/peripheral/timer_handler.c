/*
 * timer_handler.c
 *
 *  Created on: 2014/06/22
 *      Author: Yoshio
 */

#include <stddef.h>
#include "../cmsis/LPC13Uxx.h"
#include "timer.h"

#define _BV(n) ((uint32_t)0x1 << n)

typedef struct timer_event_handler
{
	timer_event_func func;
	struct timer_event_handler* next;
} timer_event_handler;

timer_event_handler timer_events_buf[TIMER_MAX_EVENT];
uint32_t timer_event_count = 0;

timer_event_handler* event_timer32_0;
timer_event_handler* event_timer32_1;

timer_event_handler event_timer_sys;

uint32_t timer32_add_event(uint32_t timer, timer_event_func func)
{
	timer_event_handler** events;
	if (timer == 0)
	{
		events = &event_timer32_0;
	}
	else
	{
		events = &event_timer32_1;
	}
	timer_event_handler* new_event = &timer_events_buf[timer_event_count];
	new_event->func = func;
	new_event->next = *events;
	*events = new_event;
	return timer_event_count++;
}

void do_timer32_0_event(uint8_t num)
{
	volatile timer_event_handler* ptr = event_timer32_0;
	while (ptr != NULL)
	{
		ptr->func(0, num);
		ptr = ptr->next;
	}
}

void do_timer32_1_event(uint8_t num)
{
	volatile timer_event_handler* ptr = event_timer32_1;
	while (ptr != NULL)
	{
		ptr->func(1, num);
		ptr = ptr->next;
	}
}

void CT32B0_IRQHandler(void)
{
	if (LPC_CT32B0->IR & _BV(0))
	{
		do_timer32_0_event(0);
		LPC_CT32B0->IR = _BV(0);
	}
	else if (LPC_CT32B0->IR & _BV(1))
	{
		do_timer32_0_event(1);
		LPC_CT32B0->IR = _BV(1);
	}
	else if (LPC_CT32B0->IR & _BV(2))
	{
		do_timer32_0_event(2);
		LPC_CT32B0->IR = _BV(2);
	}
	else if (LPC_CT32B0->IR & _BV(3))
	{
		do_timer32_0_event(3);
		LPC_CT32B0->IR = _BV(3);
	}
}

void CT32B1_IRQHandler(void)
{
	if (LPC_CT32B1->IR & _BV(0))
	{
		do_timer32_1_event(0);
		LPC_CT32B1->IR = _BV(0);
	}
	else if (LPC_CT32B1->IR & _BV(1))
	{
		do_timer32_1_event(1);
		LPC_CT32B1->IR = _BV(1);
	}
	else if (LPC_CT32B1->IR & _BV(2))
	{
		do_timer32_1_event(2);
		LPC_CT32B1->IR = _BV(2);
	}
	else if (LPC_CT32B1->IR & _BV(3))
	{
		do_timer32_1_event(3);
		LPC_CT32B1->IR = _BV(3);
	}
}

timer_event_handler* event_timer16_0;
timer_event_handler* event_timer16_1;

int timer_add_event_16_0(void (*func)(uint8_t timer, uint8_t num))
{
	timer_event_handler* new_event = &timer_events_buf[timer_event_count];
	new_event->func = func;
	new_event->next = event_timer16_0;
	event_timer16_0 = new_event;
	return timer_event_count++;
}

int timer_add_event_16_1(void (*func)(uint8_t timer, uint8_t num))
{
	timer_event_handler* new_event = &timer_events_buf[timer_event_count];
	new_event->func = func;
	new_event->next = event_timer16_1;
	event_timer16_1 = new_event;
	return timer_event_count++;
}

void do_timer16_0_event(uint8_t num)
{
	volatile timer_event_handler* ptr = event_timer16_0;
	while (ptr != NULL)
	{
		ptr->func(0, num);
		ptr = ptr->next;
	}
}

void do_timer16_1_event(uint8_t num)
{
	volatile timer_event_handler* ptr = event_timer16_1;
	while (ptr != NULL)
	{
		ptr->func(1, num);
		ptr = ptr->next;
	}
}

void CT16B0_IRQHandler(void)
{
	if (LPC_CT16B0->IR & _BV(0))
	{
		do_timer16_0_event(0);
		LPC_CT16B0->IR = _BV(0);
	}
	else if (LPC_CT16B0->IR & _BV(1))
	{
		do_timer16_0_event(1);
		LPC_CT16B0->IR = _BV(1);
	}
	else if (LPC_CT16B0->IR & _BV(2))
	{
		do_timer16_0_event(2);
		LPC_CT16B0->IR = _BV(2);
	}
	else if (LPC_CT16B0->IR & _BV(3))
	{
		do_timer16_0_event(3);
		LPC_CT16B0->IR = _BV(3);
	}
}

void CT16B1_IRQHandler(void)
{
	if (LPC_CT16B1->IR & _BV(0))
	{
		do_timer16_1_event(0);
		LPC_CT16B1->IR = _BV(0);
	}
	else if (LPC_CT16B1->IR & _BV(1))
	{
		do_timer16_1_event(1);
		LPC_CT16B1->IR = _BV(1);
	}
	else if (LPC_CT16B1->IR & _BV(2))
	{
		do_timer16_1_event(2);
		LPC_CT16B1->IR = _BV(2);
	}
	else if (LPC_CT16B1->IR & _BV(3))
	{
		do_timer16_1_event(3);
		LPC_CT16B1->IR = _BV(3);
	}
}
void add_event_system_timer(void (*func)(uint8_t timer, uint8_t num))
{
	event_timer_sys.func = func;
}

void SysTick_Handler(void)
{
	if (event_timer_sys.func != NULL)
	{
		event_timer_sys.func(0, 0);
	}
}

