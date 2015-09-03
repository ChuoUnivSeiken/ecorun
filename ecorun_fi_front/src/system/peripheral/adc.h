/*
 * adc.h
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#ifndef ADC_H_
#define ADC_H

#include "../common_types.h"

#define ADC_NUM			8
#define ADC_MAX_EVENT	1
#define ADC_MODE_BURST 1
#define ADC_ADGINTEN 0
#define ADC_MODE_10BIT 0
#define ADC_INTERRUPT 1
#define ADC_DONE              (0x80000000)
#define ADC_CHANNELS          (8)
#define ADC_CLK               (4000000) /* 4MHz */

#if defined(__cplusplus)
extern "C"
{
#endif

extern volatile uint32_t adc_done_interrupt;
extern volatile uint32_t adc_value[ADC_NUM];

int adc_add_event(void (*func)(uint8_t num, uint32_t value));

void adc_init(uint32_t clk);
uint32_t adc_read(uint8_t channel);
void adc_burst_read(void);
uint32_t* adc_get_value(void);

#if defined(__cplusplus)
}
#endif
#endif /* ADC_H_ */
