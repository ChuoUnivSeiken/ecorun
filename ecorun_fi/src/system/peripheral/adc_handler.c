/*
 * adc_handler.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC13Uxx.h"
#include "adc.h"

#define ADC_ADINT		0x00010000

typedef struct adc_event_handler
{
	void (*func)(uint8_t num, uint32_t value);
	struct adc_event_handler* next;
} adc_event_handler;

adc_event_handler adc_events_buf[ADC_MAX_EVENT];
uint32_t adc_event_count = 0;

adc_event_handler* event_adc;

volatile uint32_t adc_done_interrupt = 0;
volatile uint32_t adc_num_overrun = 0;
volatile uint32_t adc_value[ADC_NUM];
volatile uint32_t adc_burst_mask;
#if ADC_MODE_BURST
volatile uint32_t channel_flag = 0;
#endif

uint32_t* adc_get_value(void)
{
	return (uint32_t*) adc_value;
}

int adc_add_event(void (*func)(uint8_t num, uint32_t value))
{
	adc_event_handler* new_event = &adc_events_buf[adc_event_count];
	new_event->func = func;
	new_event->next = event_adc;
	event_adc = new_event;
	return adc_event_count++;
}

void do_adc_event(uint8_t num, uint32_t value)
{
	volatile adc_event_handler* ptr = event_adc;
	while (ptr != NULL)
	{
		ptr->func(num, value);
		ptr = ptr->next;
	}
}

void ADC_IRQHandler(void)
{
	volatile uint32_t regVal;
	volatile uint32_t i;
	volatile uint32_t dummy;

	regVal = LPC_ADC->STAT; /* Read ADC will clear the interrupt */

	if (regVal & 0x0000FF00) /* check OVERRUN error first */
	{
		regVal = (regVal & 0x0000FF00) >> 0x08;
		for (i = 0; i < ADC_NUM; i++)
		{
			/* if overrun, just read ADDR to clear */
			/* regVal variable has been reused. */
			if (regVal & (0x1 << i))
			{
				dummy = LPC_ADC->DR[i];
			}
		}
		LPC_ADC->CR &= ~((0x7 << 24) | (0x1 << 16)); /* stop ADC now, turn off BURST bit. */

		adc_done_interrupt = 1;
		return;
	}

	if (regVal & ADC_ADINT)
	{
		for (i = 0; i < ADC_NUM; i++)
		{
			if (regVal & (0x1 << i))
			{
#if ADC_MODE_10BIT
				adc_value[i] = ( LPC_ADC->DR[i] >> 6) & 0x3FF;
#else
				adc_value[i] = (LPC_ADC->DR[i] >> 4) & 0xFFF;
#endif
			}
		}
#if ADC_MODE_BURST
		channel_flag |= (regVal & 0xFF);
		if ((channel_flag & 0xFF) == 0xFF)
		{
			/* All the bits in have been set, it indicates all the ADC
			 channels have been converted. */
			LPC_ADC->CR &= 0xF8FeFFFF; /* stop ADC now */
#if ADC_ADGINTEN
			dummy = LPC_ADC->GDR;
#endif
			channel_flag = 0;
			adc_done_interrupt = 1;
		}
		for (i = 0; i < ADC_NUM; i++)
		{
			if (regVal & (0x1 << i))
			{
				do_adc_event(i, adc_value[i]);
			}
		}
#else
		LPC_ADC->CR &= ~(0x7 << 24); /* stop ADC now */
		adc_done_interrupt = 1;
		for (i = 0; i < ADC_NUM; i++)
		{
			if ((regVal & 0xFF) & (0x1 << i))
			{
				do_adc_event(i, adc_value[i]);
			}
		}
#endif
	}
}

