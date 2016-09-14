/*
 * timer.h
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "../common_types.h"

#define TIMER_INTERRUPT 0

#if defined(__cplusplus)
extern "C"
{
#endif

#if TIMER_INTERRUPT
#define TIMER_MAX_EVENT 5

typedef void (*timer_event_func)(uint8_t timer, uint8_t num);

uint32_t timer32_add_event(uint32_t timer, timer_event_func func);
uint32_t timer16_add_event(uint32_t timer, timer_event_func func);
#endif

void timer32_init(uint8_t timer, uint32_t interval);
void timer32_set_pwm(uint32_t timer, uint32_t interval);
void timer32_set_match(uint32_t timer, uint32_t num, uint32_t value);
void timer32_enable(uint8_t timer);
void timer32_disable(uint8_t timer);
void timer32_reset(uint8_t timer);

void timer16_init(uint8_t timer, uint32_t interval);
void timer16_set_pwm(uint32_t timer, uint32_t interval);
void timer16_set_match(uint32_t timer, uint32_t num, uint32_t value);
void timer16_enable(uint8_t timer);
void timer16_disable(uint8_t timer);
void timer16_reset(uint8_t timer);

#if defined(__cplusplus)
}
#endif

#endif /* TIMER_H_ */
