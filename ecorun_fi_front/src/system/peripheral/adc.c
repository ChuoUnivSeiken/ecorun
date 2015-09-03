/*
 * adc.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC13Uxx.h"
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

	/* Setup the ADC clock, conversion mode, etc. */
	LPC_ADC->CR = (0x01 << 0) | ((SystemCoreClock / clk - 1) << 8) | /* CLKDIV = Fpclk / 1000000 - 1 */
#if ADC_MODE_BURST
			(1 << 16) | /* BURST = 0, no BURST, software controlled */
#endif
#if ADC_MODE_LOWPOWER
			(1 << 22) | /* Low-power mode */
#endif
#if ADC_MODE_10BIT
			(1 << 23) | /* 10-bit mode */
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
uint32_t adc_read(uint8_t channel)
{
	/* channel number is 0 through 7 */
	if (channel >= ADC_NUM)
	{
		channel = 0; /* reset channel number to 0 */
	}
	LPC_ADC->CR &= 0xFFFFFF00;
	LPC_ADC->CR |= (1 << 24) | (1 << channel);
	/* switch channel,start A/D convert */

	/* Wait until the conversion is complete */
	volatile uint32_t regVal;
	while (1)
	{
		regVal = LPC_ADC->DR[channel];
		if (regVal & ADC_DONE)
		{
			break;
		}
	}

	/* Stop the ADC */
	LPC_ADC->CR &= 0xF8FFFFFF;

#if ADC_MODE_10BIT
	return ( regVal >> 6 ) & 0x3FF;
#else
	return (regVal >> 4) & 0xFFF;
#endif
}

void adc_burst_read(void)
{
	adc_done_interrupt = 0;
	if ( LPC_ADC->CR & (0x7 << 24))
	{
		LPC_ADC->CR &= ~(0x7 << 24);
	}
	/* Read all channels, 0 through 7. Be careful that if the ADCx pins is shared
	 with SWD CLK or SWD IO. */
	LPC_ADC->CR |= (_BV(1) | _BV(2) | _BV(3));
	LPC_ADC->CR |= (0x1 << 16); /* Set burst mode and start A/D convert */
	return; /* the ADC reading is done inside the
	 handler, return 0. */
}

