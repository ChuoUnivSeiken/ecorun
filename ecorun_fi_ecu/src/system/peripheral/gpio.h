/*
 * gpio.h
 *
 *  Created on: 2014/06/23
 *      Author: Yoshio
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "../common_types.h"

#define ENABLE_PIOINT0_IRQHandler 1
#define ENABLE_PIOINT1_IRQHandler 1
#define ENABLE_PIOINT2_IRQHandler 0
#define ENABLE_PIOINT3_IRQHandler 0

typedef struct gpio_event_handler
{
	void (*func)(uint8_t port, uint8_t bit);
	struct gpio_event_handler* next;
} gpio_event_handler;

int gpio0_add_event(void (*func)(uint8_t port, uint8_t bit));
int gpio1_add_event(void (*func)(uint8_t port, uint8_t bit));

#define PORT0		0
#define PORT1		1
#define PORT2		2
#define PORT3		3

void gpio_init(void);
void gpio_set_value(uint32_t port, uint32_t bit, uint32_t value);
uint32_t gpio_get_value(uint32_t port, uint32_t bit);
void gpio_set_dir(uint32_t port, uint32_t bit, uint32_t value);
void gpio_enable_interrupt(uint32_t port, uint32_t bit);
void gpio_disable_interrupt(uint32_t port, uint32_t bit);
uint32_t gpio_has_interrupted(uint32_t port, uint32_t bit);
void gpio_clear_interrupted(uint32_t port, uint32_t bit);

/*
 * #define PORT0		0
 #define PORT1		1
 #define PORT2		2
 #define PORT3		3
 static LPC_GPIO_TypeDef (* const LPC_GPIO[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };
 void PIOINT0_IRQHandler(void);
 void PIOINT1_IRQHandler(void);
 void PIOINT2_IRQHandler(void);
 void PIOINT3_IRQHandler(void);
 void GPIOInit( void );
 void GPIOSetInterrupt( uint32_t portNum, uint32_t bitPosi, uint32_t sense,
 uint32_t single, uint32_t event );

 void GPIOIntEnable( uint32_t portNum, uint32_t bitPosi );
 void GPIOIntDisable( uint32_t portNum, uint32_t bitPosi );
 uint32_t GPIOIntStatus( uint32_t portNum, uint32_t bitPosi );
 void GPIOIntClear( uint32_t portNum, uint32_t bitPosi );
 void GPIOSetValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal );
 void GPIOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir );
 *
 */

#endif /* GPIO_H_ */
