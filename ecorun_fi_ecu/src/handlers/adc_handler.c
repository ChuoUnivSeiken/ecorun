/*
 * adc_handler.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../system/cmsis/LPC11xx.h"
#include "../system/peripheral/adc.h"
#include <stdlib.h>

#define ADC_DONE		0x80000000
#define ADC_OVERRUN		0x40000000
#define ADC_ADINT		0x00010000

volatile uint32_t adc_done_interrupt = 0;
volatile uint32_t adc_num_overrun = 0;
volatile uint32_t adc_value[ADC_NUM];

#if ADC_MODE_BURST
volatile uint32_t channel_flag = 0;
#endif

uint32_t adc_get_value(uint8_t channel)
{
	return adc_value[channel];
}

void ADC_IRQHandler(void)
{
	volatile uint32_t regval;
	volatile uint32_t i;
	volatile uint32_t dummy;

	regval = LPC_ADC->STAT; /* Read ADC will clear the interrupt */

	if (regval & 0x0000FF00) /* check OVERRUN error first */
	{
		adc_num_overrun++;
		regval = (regval & 0x0000FF00) >> 0x08;
		for (i = 0; i < ADC_NUM; i++)
		{
			/* if overrun, just read ADDR to clear */
			/* regVal variable has been reused. */
			if (regval & (0x1 << i))
			{
				dummy = LPC_ADC->DR[i];
			}
		}
		LPC_ADC->CR &= 0xF8FeFFFF; /* stop ADC now, turn off BURST bit. */

		adc_done_interrupt = 1;
		return;
	}

	if (regval & ADC_ADINT)
	{
		for (i = 0; i < ADC_NUM; i++)
		{
			if (regval & (0x1 << i))
			{
				adc_value[i] = (LPC_ADC->DR[i] >> 6) & 0x3FF;
			}
		}
#if ADC_MODE_BURST
		channel_flag |= (regval & 0xFF);
		if ((channel_flag & ADC_CHANNEL_MASK) == ADC_CHANNEL_MASK)
		{
			/* All the bits in have been set, it indicates all the ADC
			 channels have been converted. */
			LPC_ADC->CR &= 0xF8FeFFFF; /* stop ADC now */

			channel_flag = 0;
			adc_done_interrupt = 1;
		}
#else
		LPC_ADC->CR &= 0xF8FFFFFF; /* stop ADC now */

		adc_done_interrupt = 1;
#endif
	}
}

