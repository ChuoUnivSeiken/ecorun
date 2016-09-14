/*
 * gpio_handler.c
 *
 *  Created on: 2014/06/22
 *      Author: Yoshio
 */

#include "../system/cmsis/LPC11xx.h"
#include "../system/peripheral/gpio.h"

gpio_event_handler events_buf[20];
uint32_t gpio_event_count = 0;

#if ENABLE_PIOINT0_IRQHandler

gpio_event_handler* event_gpio0;

int gpio0_add_event(void (*func)(uint8_t port, uint8_t bit))
{
	gpio_event_handler* new_event = &events_buf[gpio_event_count];
	new_event->func = func;
	new_event->next = event_gpio0;
	event_gpio0 = new_event;
	return gpio_event_count++;
}

void do_gpio0_event(uint8_t bit)
{
	volatile gpio_event_handler* ptr = event_gpio0;
	while (ptr != NULL)
	{
		ptr->func(0, bit);
		ptr = ptr->next;
	}
}

void PIOINT0_IRQHandler()
{
	volatile uint32_t i;
	for (i = 0; i < 12; i++)
	{
		if (gpio_has_interrupted(PORT0, i))
		{
			do_gpio0_event(i);
			gpio_clear_interrupted(PORT0, i);
		}
	}
}
#endif

#if ENABLE_PIOINT1_IRQHandler

gpio_event_handler* event_gpio1;

int gpio1_add_event(void (*func)(uint8_t port, uint8_t bit))
{
	gpio_event_handler* new_event = &events_buf[gpio_event_count];
	new_event->func = func;
	new_event->next = event_gpio1;
	event_gpio1 = new_event;
	return gpio_event_count++;
}

void do_gpio1_event(uint8_t bit)
{
	volatile gpio_event_handler* ptr = event_gpio1;
	while (ptr != NULL)
	{
		ptr->func(1, bit);
		ptr = ptr->next;
	}
}

void PIOINT1_IRQHandler()
{
	volatile int i;
	for (i = 0; i < 12; i++)
	{
		if (LPC_GPIO1->MIS & (1 << i))
		{
			do_gpio1_event(i);
			LPC_GPIO1->IC = (1 << i);
		}
	}
}
#endif

#if ENABLE_PIOINT2_IRQHandler

gpio_event_handler* event_gpio2;
int gpio2_add_event(void (*func)(uint8_t port, uint8_t bit))
{
	gpio_event_handler* new_event = &events_buf[gpio_event_count];
	new_event->func = func;
	new_event->next = event_gpio2;
	event_gpio2 = new_event;
	return gpio_event_count++;
}

void do_gpio2_event(uint8_t bit)
{
	volatile gpio_event_handler* ptr = event_gpio2;
	while (ptr != NULL)
	{
		ptr->func(2, bit);
		ptr = ptr->next;
	}
}

void PIOINT2_IRQHandler()
{
	volatile int i;
	for (i = 0; i < 12; i++)
	{
		if (LPC_GPIO2->MIS & (1 << i))
		{
			do_gpio2_event(i);
			LPC_GPIO2->IC = (1 << i);
		}
	}
}
#endif

#if ENABLE_PIOINT3_IRQHandler

void PIOINT2_IRQHandler()
{
	volatile int i;
	for (i = 0; i < 12; i++)
	{
		if (LPC_GPIO3->MIS & (1 << i))
		{
			do_gpio2_event(i);
			LPC_GPIO3->IC = (1 << i);
		}
	}
}

gpio_event_handler* event_gpio3;

int gpio3_add_event(void (*func)(uint8_t port, uint8_t bit))
{
	gpio_event_handler* new_event = &events_buf[gpio_event_count];
	new_event->func = func;
	new_event->next = event_gpio3;
	event_gpio3 = new_event;
	return gpio_event_count++;
}

void do_gpio3_event(uint8_t bit)
{
	volatile gpio_event_handler* ptr = event_gpio3;
	while (ptr != NULL)
	{
		ptr->func(3, bit);
		ptr = ptr->next;
	}
}
#endif
