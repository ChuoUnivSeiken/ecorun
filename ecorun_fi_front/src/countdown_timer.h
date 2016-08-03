/*
 * countdown_timer.h
 *
 *  Created on: 2015/10/08
 *      Author: Yoshio
 */

#ifndef COUNTDOWN_TIMER_H_
#define COUNTDOWN_TIMER_H_

#include <stdint.h>

volatile uint32_t countdown_timer_val;

void countdown_timer_init(void);
void delay_ms(uint32_t ms);
void start_countdown(uint32_t ms);

#endif /* COUNTDOWN_TIMER_H_ */
