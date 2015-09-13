/*
 * gpio.c
 *
 *  Created on: 2014/06/23
 *      Author: Yoshio
 */

#include "../cmsis/LPC11xx.h"
#include "gpio.h"

void gpio_init(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6);
#if ENABLE_PIOINT0_IRQHandler
	NVIC_EnableIRQ(EINT0_IRQn);
#endif
#if ENABLE_PIOINT1_IRQHandler
	NVIC_EnableIRQ(EINT1_IRQn);
#endif
#if ENABLE_PIOINT2_IRQHandler
	NVIC_EnableIRQ(EINT2_IRQn);
#endif
#if ENABLE_PIOINT3_IRQHandler
	NVIC_EnableIRQ(EINT3_IRQn);
#endif
}

LPC_GPIO_TypeDef* LPC_GPIO[] = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };

void gpio_set_value(uint32_t port, uint32_t bit, uint32_t value) {
	if (value)
		LPC_GPIO[port]->DATA |= _BV(bit);
	else
		LPC_GPIO[port]->DATA &= ~_BV(bit);
}

void gpio_set_dir(uint32_t port, uint32_t bit, uint32_t value) {
	if (value)
		LPC_GPIO[port]->DIR |= _BV(bit);
	else
		LPC_GPIO[port]->DIR &= ~_BV(bit);
}

void gpio_enable_interrupt(uint32_t port, uint32_t bit) {
	LPC_GPIO[port]->IE |= _BV(bit);
}

void gpio_disable_interrupt(uint32_t port, uint32_t bit) {
	LPC_GPIO[port]->IE &= ~_BV(bit);
}

uint32_t gpio_has_interrupted(uint32_t port, uint32_t bit) {
	return LPC_GPIO[port]->MIS & _BV(bit);
}

void gpio_clear_interrupted(uint32_t port, uint32_t bit) {
	LPC_GPIO0->IC = _BV(bit);
}

