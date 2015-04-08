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
//#define ADC_CLK			12000000		/* set to 4Mhz */
#define ADC_CLK			100000
#define ADC_MAX_EVENT	1
#define ADC_BURST_MODE 0
#define ADC_ADGINTEN 0
#define ADC_MODE_10BIT 0
#define ADC_INTERRUPT 0

#if defined(__cplusplus)
extern "C" {
#endif

extern volatile uint32_t adc_done_interrupt;
extern volatile uint32_t adc_value[ADC_NUM];

int adc_add_event(void (*func)(uint8_t num, uint32_t value));

void adc_init(uint32_t clk);
void adc_read(uint8_t channel);
void adc_read_mask(uint8_t mask);
void adc_burst_read(void);
void adc_burst_read_channel(uint32_t channel);
uint32_t* adc_get_value(void);

#if defined(__cplusplus)
}
#endif
#endif /* ADC_H_ */
