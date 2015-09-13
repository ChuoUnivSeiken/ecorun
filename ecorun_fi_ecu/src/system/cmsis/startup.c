/*================================*/
/*=====LPC11XX GNU STARTUP========*/
/*==A CODERED COMPATIBLE STARTUP==*/
/*================================*/
#if defined (__cplusplus)
#ifdef __REDLIB__
#error Redlib does not support C++
#else
//*****************************************************************************
//
// The entry point for the C++ library startup
//
//*****************************************************************************
extern "C"
{
	extern void __libc_init_array(void);
}
#endif
#endif

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

#include "LPC11xx.h"

//*****************************************************************************
#if defined (__cplusplus)
extern "C"
{
#endif

#ifdef FIXED_STACKHEAP_SIZE
#define STACK_SIZE  (800)
#define HEAP_SIZE  (200)
unsigned char StackMem[STACK_SIZE] __attribute__ ((section(".stack")));
unsigned char HeapMem[HEAP_SIZE] __attribute__ ((section(".heap"), align(8)));
#endif
//*****************************************************************************
//
// Forward declaration of the default handlers. These are aliased.
// When the application defines a handler (with the same name), this will
// automatically take precedence over these weak definitions
//
//*****************************************************************************
void Reset_Handler(void);
WEAK void NMI_Handler(void);
WEAK void HardFault_Handler(void);
WEAK void SVCall_Handler(void);
WEAK void PendSV_Handler(void);
WEAK void SysTick_Handler(void);
WEAK void IntDefaultHandler(void);

//*****************************************************************************
//
// Forward declaration of the specific IRQ handlers. These are aliased
// to the IntDefaultHandler, which is a 'forever' loop. When the application
// defines a handler (with the same name), this will automatically take
// precedence over these weak definitions
//
//*****************************************************************************
void CAN_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP1_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER16_0_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER16_1_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER32_0_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER32_1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP0_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART_IRQHandler(void) ALIAS(IntDefaultHandler);
void ADC_IRQHandler(void) ALIAS(IntDefaultHandler);
void WDT_IRQHandler(void) ALIAS(IntDefaultHandler);
void BOD_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT3_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT2_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT0_IRQHandler(void) ALIAS(IntDefaultHandler);
void WAKEUP_IRQHandler(void) ALIAS(IntDefaultHandler);

//*****************************************************************************
//
// The entry point for the application.
// __main() is the entry point for Redlib based applications
// main() is the entry point for Newlib based applications
//
//*****************************************************************************
#if defined (__REDLIB__)
extern void __main(void);
#else
extern int main(void);
#endif

extern unsigned int _estack;

//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000.
//
//*****************************************************************************
__attribute__ ((section(".isr_vector")))
void (* const Vectors[])(void) =
{
#ifdef FIXED_STACKHEAP_SIZE
		(void (*)(void))(StackMem + STACK_SIZE), // The initial stack pointer
#else
		(void (*)(void))&_estack,
#endif
		Reset_Handler,                               // The reset handler
		NMI_Handler,//NMI_Handler,
		HardFault_Handler,// The hard fault handler
		0,			// Reserved
		0,			// Reserved
		0,			// Reserved
		0,			// Reserved
		0,			// Reserved
		0,			// Reserved
		0,			// Reserved
		SVCall_Handler,// SVCall handler
		0,			// Reserved
		0,			// Reserved
		PendSV_Handler,// The PendSV handler
		SysTick_Handler,// The SysTick handler

		// LPC11XX External Interrupts
		WAKEUP_IRQHandler,// PIO0_0  Wakeup
		WAKEUP_IRQHandler,// PIO0_1  Wakeup
		WAKEUP_IRQHandler,// PIO0_2  Wakeup
		WAKEUP_IRQHandler,// PIO0_3  Wakeup
		WAKEUP_IRQHandler,// PIO0_4  Wakeup
		WAKEUP_IRQHandler,// PIO0_5  Wakeup
		WAKEUP_IRQHandler,// PIO0_6  Wakeup
		WAKEUP_IRQHandler,// PIO0_7  Wakeup
		WAKEUP_IRQHandler,// PIO0_8  Wakeup
		WAKEUP_IRQHandler,// PIO0_9  Wakeup
		WAKEUP_IRQHandler,// PIO0_10 Wakeup
		WAKEUP_IRQHandler,// PIO0_11 Wakeup
		WAKEUP_IRQHandler,// PIO1_0  Wakeup
		CAN_IRQHandler,// C_CAN Interrupt
		SSP1_IRQHandler,// SPI/SSP1 Interrupt
		I2C_IRQHandler,// I2C0
		TIMER16_0_IRQHandler,// CT16B0 (16-bit Timer 0)
		TIMER16_1_IRQHandler,// CT16B1 (16-bit Timer 1)
		TIMER32_0_IRQHandler,// CT32B0 (32-bit Timer 0)
		TIMER32_1_IRQHandler,// CT32B1 (32-bit Timer 1)
		SSP0_IRQHandler,// SPI/SSP0 Interrupt
		UART_IRQHandler,// UART0
		0,// Reserved
		0,// Reserved
		ADC_IRQHandler,// ADC   (A/D Converter)
		WDT_IRQHandler,// WDT   (Watchdog Timer)
		BOD_IRQHandler,// BOD   (Brownout Detect)
		0,// Reserved
		PIOINT3_IRQHandler,// PIO INT3
		PIOINT2_IRQHandler,// PIO INT2
		PIOINT1_IRQHandler,// PIO INT1
		PIOINT0_IRQHandler,// PIO INT0
};

// Begin address for the initialisation values of the .data section.
// defined in linker script
extern unsigned int _sidata;
// Begin address for the .data section; defined in linker script
extern unsigned int _sdata;
// End address for the .data section; defined in linker script
extern unsigned int _edata;

// Begin address for the .bss section; defined in linker script
extern unsigned int __bss_start__;
// End address for the .bss section; defined in linker script
extern unsigned int __bss_end__;

inline void
__attribute__((always_inline))
__initialize_data(unsigned int* from, unsigned int* section_begin,
		unsigned int* section_end)
{
	// Iterate and copy word by word.
	// It is assumed that the pointers are word aligned.
	unsigned int *p = section_begin;
	while (p < section_end)
		*p++ = *from++;
}

inline void
__attribute__((always_inline))
__initialize_bss(unsigned int* section_begin, unsigned int* section_end)
{
	// Iterate and clear word by word.
	// It is assumed that the pointers are word aligned.
	unsigned int *p = section_begin;
	while (p < section_end)
		*p++ = 0;
}

// These magic symbols are provided by the linker.
extern void
(*__preinit_array_start[])(void) __attribute__((weak));
extern void
(*__preinit_array_end[])(void) __attribute__((weak));
extern void
(*__init_array_start[])(void) __attribute__((weak));
extern void
(*__init_array_end[])(void) __attribute__((weak));
extern void
(*__fini_array_start[])(void) __attribute__((weak));
extern void
(*__fini_array_end[])(void) __attribute__((weak));

// Iterate over all the preinit/init routines (mainly static constructors).
inline void
__attribute__((always_inline))
__run_init_array(void)
{
	int count;
	int i;

	count = __preinit_array_end - __preinit_array_start;
	for (i = 0; i < count; i++)
		__preinit_array_start[i]();

	// If you need to run the code in the .init section, please use
	// the startup files, since this requires the code in crti.o and crtn.o
	// to add the function prologue/epilogue.
	//_init(); // DO NOT ENABE THIS!

	count = __init_array_end - __init_array_start;
	for (i = 0; i < count; i++)
		__init_array_start[i]();
}

// Run all the cleanup routines (mainly static destructors).
inline void
__attribute__((always_inline))
__run_fini_array(void)
{
	int count;
	int i;

	count = __fini_array_end - __fini_array_start;
	for (i = count; i > 0; i--)
		__fini_array_start[i - 1]();

	// If you need to run the code in the .fini section, please use
	// the startup files, since this requires the code in crti.o and crtn.o
	// to add the function prologue/epilogue.
	//_fini(); // DO NOT ENABE THIS!
}

void Reset_Handler(void)
{
	// Zero fill the bss segment
	__initialize_bss(&__bss_start__, &__bss_end__);

	// Copy the data segment from Flash to RAM.
	// When using startup files, this code is executed via the preinit array.
	__initialize_data(&_sidata, &_sdata, &_edata);

	long *s, *d;

	/* Configure BOD control (Reset on Vcc dips below 2.7V) */
	LPC_SYSCON->BODCTRL = 0x13;

	/* Configure system clock generator (36MHz system clock with IRC) */
	LPC_SYSCON->MAINCLKSEL = 0;
	LPC_SYSCON->MAINCLKUEN = 0;
	LPC_SYSCON->MAINCLKUEN = 1;
	LPC_FLASHCTRL->FLASHCFG = (LPC_FLASHCTRL->FLASHCFG & 0xFFFFFFFC) | 1; /* Set wait state for flash memory (1WS) */

	LPC_SYSCON->SYSPLLCLKSEL = 0; /* Select IRC for PLL-in */
	LPC_SYSCON->SYSPLLCLKUEN = 0;
	LPC_SYSCON->SYSPLLCLKUEN = 1;
	LPC_SYSCON->SYSPLLCTRL = (4 - 1) | (1 << 6); /* Set PLL parameters (M=4, P=2) */
	LPC_SYSCON->PDRUNCFG &= ~0x80; /* Enable PLL */
	while ((LPC_SYSCON->SYSPLLSTAT & 1) == 0)
		; /* Wait for PLL locked */
	LPC_SYSCON->SYSAHBCLKDIV = 1; /* Set system clock divisor (1) */
	LPC_SYSCON->MAINCLKSEL = 3; /* Select PLL-out as main clock */
	LPC_SYSCON->MAINCLKUEN = 0;
	LPC_SYSCON->MAINCLKUEN = 1;

	/* Enable clock for only SYS, ROM, RAM, FLASH, GPIO and IOCON */
	LPC_SYSCON->SYSAHBCLKCTRL = 0x1005F;

	// Call the standard library initialisation (mandatory for C++ to
	// execute the constructors for the static objects).
	__run_init_array();

	// Call the main entry point, and save the exit code.
	int code = main();

	// Run the C++ static destructors.
	__run_fini_array();

	_exit(code);

	// Should never reach this, _exit() should have already
	// performed a reset.
	for (;;)
		;
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void NMI_Handler(void)
{
	while (1)
	{
	}
}

__attribute__ ((section(".after_vectors")))
void HardFault_Handler(void)
{
	while (1)
	{
	}
}

__attribute__ ((section(".after_vectors")))
void SVCall_Handler(void)
{
	while (1)
	{
	}
}

__attribute__ ((section(".after_vectors")))
void PendSV_Handler(void)
{
	while (1)
	{
	}
}

#if 0
__attribute__ ((section(".after_vectors")))
void SysTick_Handler(void)
{
	while (1)
	{
	}
}
#endif

//*****************************************************************************
//
// Processor ends up here if an unexpected interrupt occurs or a handler
// is not present in the application code.
//
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void IntDefaultHandler(void)
{
	while (1)
	{
	}
}

