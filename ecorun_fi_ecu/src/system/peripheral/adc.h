/*
 * adc.h
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#ifndef ADC_H_
#define ADC_H

#include "../common_types.h"

#define ADC_NUM			8			/* for LPC11xx */
#define ADC_CLK			4000000		/* set to 1Mhz */
#define ADC_MAX_EVENT	1
#define ADC_MODE_BURST	1
#define ADC_INTERRUPT	1
#define ADC_ADGINTEN	0

#define ADC_CHANNEL_MASK		((0x1 << 1) | (0x1 << 2) | (0x1 << 3))

int adc_add_event(void (*func)(uint8_t num, uint32_t value));

void adc_init(uint32_t clk);
void adc_read(uint8_t channel);
void adc_burst_read(void);
uint32_t* adc_get_value(void);

#endif /* ADC_H_ */
