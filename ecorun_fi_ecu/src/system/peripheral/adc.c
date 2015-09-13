/*
 * adc.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "adc.h"

extern volatile uint32_t adc_value[ADC_NUM];

void adc_init(uint32_t clk)
{
	volatile uint32_t i;

	/* Disable Power down bit to the ADC block. */
	LPC_SYSCON->PDRUNCFG &= ~(0x1 << 4);

	/* Enable AHB clock to the ADC. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13);

	for (i = 0; i < ADC_NUM; i++)
	{
		adc_value[i] = 0x0;
	}

	LPC_ADC->CR = ((((SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / clk) - 1)
			<< 8) |
#if ADC_MODE_BURST
			(1 << 16) |
#endif
			(0 << 24) | /* START = 0 A/D conversion stops */
			(0 << 27); /* EDGE = 0 (CAP/MAT rising edge, trigger A/D conversion) */

#if ADC_INTERRUPT
	/* If POLLING, no need to do the following */
	NVIC_EnableIRQ(ADC_IRQn);
	LPC_ADC->INTEN = 0xFF; /* Enable all interrupts */
#if (ADC_ADGINTEN && !ADC_MODE_BURST)
	LPC_ADC->INTEN |= 0x100;
#endif
#endif
	return;
}

void adc_read(uint8_t channel)
{
	/* channel number is 0 through 7 */
	if (channel >= ADC_NUM)
	{
		channel = 0; /* reset channel number to 0 */
	}
	LPC_ADC->CR &= 0xFFFFFF00;
	LPC_ADC->CR |= (1 << 24) | (1 << channel);
	/* switch channel,start A/D convert */
}

extern volatile uint32_t adc_done_interrupt;

void adc_burst_read(void)
{
	adc_done_interrupt = 0;
	if ( LPC_ADC->CR & (0x7 << 24))
	{
		LPC_ADC->CR &= ~(0x7 << 24);
	}
	/* Read all channels, 0 through 7. Be careful that if the ADCx pins is shared
	 with SWD CLK or SWD IO. */
	LPC_ADC->CR |= ADC_CHANNEL_MASK;
	LPC_ADC->CR |= (0x1 << 16); /* Set burst mode and start A/D convert */
	return; /* the ADC reading is done inside the
	 handler, return 0. */
}

