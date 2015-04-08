/*
 * adc.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "../cmsis/LPC13Uxx.h"
#include "adc.h"

extern volatile uint32_t adc_value[ADC_NUM];
extern volatile uint32_t adc_burst_mask;

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

	/* P0.11 = ADC0 */
	LPC_IOCON->TDI_PIO0_11 &= ~0x9F;
	LPC_IOCON->TDI_PIO0_11 |= 0x02;
	/* P0.12 = ADC1 */
	LPC_IOCON->TMS_PIO0_12 &= ~0x9F;
	LPC_IOCON->TMS_PIO0_12 |= 0x02;
	/* P0.13 = ADC2 */
	LPC_IOCON->TDO_PIO0_13 &= ~0x9F;
	LPC_IOCON->TDO_PIO0_13 |= 0x02;
	/* P0.14 = ADC3 */
	LPC_IOCON->TRST_PIO0_14 &= ~0x9F;
	LPC_IOCON->TRST_PIO0_14 |= 0x02;
	/* P0.15 = ADC4 ... this is also SWDIO so be careful with this pin! */
	LPC_IOCON->SWDIO_PIO0_15 &= ~0x9F;
	LPC_IOCON->SWDIO_PIO0_15 |= 0x02;
	/* P0.16 = ADC5 */
	LPC_IOCON->PIO0_16 &= ~0x9F;
	LPC_IOCON->PIO0_16 |= 0x01;
	/* P0.22 = ADC6 */
	LPC_IOCON->PIO0_22 &= ~0x9F;
	LPC_IOCON->PIO0_22 |= 0x01;
	/* P0.23 = ADC7 */
	LPC_IOCON->PIO0_23 &= ~0x9F;
	LPC_IOCON->PIO0_23 |= 0x01;
	/* Setup the ADC clock, conversion mode, etc. */
	LPC_ADC->CR = (0x01 << 0) | ((SystemCoreClock / clk - 1) << 8) | /* CLKDIV = Fpclk / 1000000 - 1 */
	(0 << 16) | /* BURST = 0, no BURST, software controlled */
	(0 << 17) | /* CLKS = 0, 11 clocks/10 bits */
#if ADC_MODE_LOWPOWER
			(1 << 22) | /* Low-power mode */
#endif
#if ADC_MODE_10BIT
			(1 << 23) | /* 10-bit mode */
#endif
			(0 << 24) | /* START = 0 A/D conversion stops */
			(0 << 27); /* EDGE = 0 (CAP/MAT rising edge, trigger A/D conversion) */

#ifdef ADC_INTERRUP
	/* If POLLING, no need to do the following */
	NVIC_EnableIRQ(ADC_IRQn);
	LPC_ADC->INTEN = 0xFF; /* Enable all interrupts */
#if ADC_ADGINTEN
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
void adc_read_mask(uint8_t mask)
{
	LPC_ADC->CR &= 0xFFFFFF00;
	LPC_ADC->CR |= (1 << 24) | mask;
	/* switch channel,start A/D convert */
}
void adc_burst_read(void)
{
	if ( LPC_ADC->CR & (0x7 << 24))
	{
		LPC_ADC->CR &= ~(0x7 << 24);
	}
	/* Read all channels, 0 through 7. Be careful that if the ADCx pins is shared
	 with SWD CLK or SWD IO. */
	LPC_ADC->CR |= (0xFF);
	LPC_ADC->CR |= (0x1 << 16); /* Set burst mode and start A/D convert */
	return; /* the ADC reading is done inside the
	 handler, return 0. */
}
void adc_burst_read_channel(uint32_t channel)
{
	adc_burst_mask = channel;
	if ( LPC_ADC->CR & (0x7 << 24))
	{
		LPC_ADC->CR &= ~(0x7 << 24);
	}
	/* Read all channels, 0 through 7. Be careful that if the ADCx pins is shared
	 with SWD CLK or SWD IO. */
	LPC_ADC->CR |= channel;
	LPC_ADC->CR |= (0x1 << 16); /* Set burst mode and start A/D convert */
	return; /* the ADC reading is done inside the
	 handler, return 0. */
}

