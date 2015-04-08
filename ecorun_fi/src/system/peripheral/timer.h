/*
 * timer.h
 *
 *  Created on: 2014/06/26
 *      Author: Yoshio
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "../common_types.h"

#define ENABLE_INTERRUPT 1

#if defined(__cplusplus)
extern "C"
{
#endif

#if ENABLE_INTERRUPT
#define TIMER_MAX_EVENT 5

typedef void (*timer_event_func)(uint8_t timer, uint8_t num);

uint32_t timer32_add_event(uint32_t timer, timer_event_func func);
#endif

void timer32_init(uint32_t timer, uint32_t interval);
void timer32_set_pwm(uint32_t timer, uint32_t interval);
void timer32_set_match(uint32_t timer, uint32_t num, uint32_t value);
void timer32_enable(uint32_t timer);
void timer32_disable(uint32_t timer);
void timer32_reset(uint32_t timer);

int timer_add_event_16_0(void (*func)(uint8_t timer, uint8_t num));
int timer_add_event_16_1(void (*func)(uint8_t timer, uint8_t num));

void enable_timer16_0(void);
void enable_timer16_1(void);
void disable_timer16_0(void);
void disable_timer16_1(void);
void reset_timer16_0(void);
void reset_timer16_1(void);
void init_timer16_0(uint32_t prescale, uint32_t interval);
void init_timer16_1(uint32_t prescale, uint32_t interval);

#if defined(__cplusplus)
}
#endif

#endif /* TIMER_H_ */
