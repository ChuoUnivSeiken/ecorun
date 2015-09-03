/*
 * timer.h
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "../type.h"

#define TIMER_MAX_EVENT 5

typedef struct timer_event_handler
{
	void (*func)(uint8_t timer, uint8_t num);
	struct timer_event_handler* next;
} timer_event_handler;

int timer_add_event_32_0(void (*func)(uint8_t timer, uint8_t num));
int timer_add_event_32_1(void (*func)(uint8_t timer, uint8_t num));

void timer32_enable(uint8_t timer);
void timer32_disable(uint8_t timer);
void timer32_reset(uint8_t timer);
void timer32_init(uint8_t timer, uint32_t interval);

int timer_add_event_16_0(void (*func)(uint8_t timer, uint8_t num));
int timer_add_event_16_1(void (*func)(uint8_t timer, uint8_t num));

void timer16_enable(uint8_t timer);
void timer16_disable(uint8_t timer);
void timer16_reset(uint8_t timer);
void timer16_init(uint8_t timer, uint32_t prescale, uint32_t interval);
void timer16_set_pwm(uint32_t timer, uint32_t interval);
void timer16_set_match(uint32_t timer, uint32_t num, uint32_t value);

void enable_system_timer(uint32_t fraq);

#endif /* TIMER_H_ */
