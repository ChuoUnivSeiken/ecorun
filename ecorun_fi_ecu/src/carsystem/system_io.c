#include "system_io.h"

#include "../system/cmsis/LPC11xx.h"

void set_starter_sw(bool on)
{
	if (on)
	{
		LPC_GPIO1->DATA |= _BV(11);
	}
	else
	{
		LPC_GPIO1->DATA &= ~_BV(11);
	}
}

void set_cdi_sw(bool on)
{
	if (on)
	{
		LPC_GPIO1->DATA |= _BV(4);
	}
	else
	{
		LPC_GPIO1->DATA &= ~_BV(4);
	}
}

void set_fuel_sw(bool on)
{
	if (on)
	{
		LPC_GPIO1->DATA |= _BV(3);
	}
	else
	{
		LPC_GPIO1->DATA &= ~_BV(3);
	}
}

uint32_t get_af(void)
{
	return (LPC_GPIO2->DATA & _BV(10)) != 0;
}

void init_io_input_signals(void)
{
	LPC_IOCON->PIO2_10 = 0xd0; // af
	LPC_GPIO2->DIR &= ~_BV(10);
}

void init_io_output_signals(void)
{
	LPC_IOCON->SWDIO_PIO1_3 &= ~0x07; // fuel
	LPC_IOCON->SWDIO_PIO1_3 |= 0x01;

	LPC_IOCON->PIO1_4 &= ~0x07; // cdi
	LPC_IOCON->PIO1_4 |= 0x00;

	LPC_IOCON->PIO1_11 &= ~0x07; // starter
	LPC_IOCON->PIO1_11 |= 0x00;

	LPC_GPIO1->DATA &= ~_BV(3);
	LPC_GPIO1->DATA &= ~_BV(4);
	LPC_GPIO1->DATA &= ~_BV(11);

	LPC_GPIO1->DIR |= _BV(3);
	LPC_GPIO1->DIR |= _BV(4);
	LPC_GPIO1->DIR |= _BV(11);
}

void init_io(void)
{
	LPC_IOCON->R_PIO1_0 = 0x02; // adc 1 for throttle angle
	LPC_IOCON->R_PIO1_1 = 0x02; // adc 2 for throttle angle
	LPC_IOCON->R_PIO1_2 = 0x02; // adc 3 for throttle angle
	LPC_IOCON->SWCLK_PIO0_10 = 0xd3; // fi clock
	init_io_output_signals();
}
