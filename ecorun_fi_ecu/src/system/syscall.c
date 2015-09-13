/*
 * syscall.c
 *
 *  Created on: 2014/06/27
 *      Author: Yoshio
 */

#define __CC_ARM
#include "cmsis/LPC11xx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/reent.h>
#include "peripheral/uart.h"
//#include "lpc17xx_uart.h"

#define ECHOBACK

extern int errno;
extern long _sidata[], _sdata[], _edata[], _sbss[], _ebss[], _endof_sram[];
extern long _heap_start[], _heap_end[], _end[];
unsigned char* heap_end;
register unsigned char* stack_ptr asm ("sp");

int _read_r(struct _reent *r, int file, char *ptr, int len) {
	uart_puts_with_term("_read_r");
	int i;
	unsigned char *p = (unsigned char*) ptr;

	for (i = 0; i < len; i++) {
		//UART_Receive(Uart, p, 1, BLOCKING);

#ifdef ECHOBACK
		//UART_Send(Uart, p, 1, BLOCKING);
#endif

		if (*p++ == '\r' && i <= (len - 2)) /* 0x0D */
		{
			*p = '\n'; /* 0x0A */
#ifdef ECHOBACK
			//UART_Send(Uart, p, 1, BLOCKING); /* 0x0A */
#endif
			return i + 2;
		}
	}
	return i;
}

int _lseek_r(struct _reent *r, int file, int ptr, int dir) {
	uart_puts_with_term("_lseek_r");
	return 0;
}

int _write_r(struct _reent *r, int file, const void *ptr, size_t len) {
	uart_puts_with_term("_write_r");
	int i;
	unsigned char *p = (unsigned char*) ptr;

	for (i = 0; i < len; i++) {
		if (*p == '\n') {
			//UART_Send(Uart, (uint8_t*) '\r', 1, BLOCKING);
		}
		//UART_Send(Uart, p++, 1, BLOCKING);
	}
	return len;
}

int _close_r(struct _reent *r, int file) {
	uart_puts_with_term("_close_r");
	return -1;
}

caddr_t _sbrk_r(struct _reent *r, int incr) {
	uart_puts_with_term("_sbrk_r\n");
	uart_puts_with_term("stack_ptr:");
	uart_uint32_with_term((uint32_t) stack_ptr);
	uart_puts_with_term("\n");
	uart_puts_with_term("heap_end:");
	uart_uint32_with_term((uint32_t) heap_end);
	uart_puts_with_term("\n");
	uart_puts_with_term("incr:");
	uart_uint32_with_term((uint32_t) incr);
	uart_puts_with_term("\n");
	uart_puts_with_term("_heap_start:");
	uart_uint32_with_term((uint32_t) _heap_start);
	uart_puts_with_term("\n");
	uart_puts_with_term("_heap_end:");
	uart_uint32_with_term((uint32_t) _heap_end);
	uart_puts_with_term("\n");
	uart_puts_with_term("_end:");
	uart_uint32_with_term((uint32_t) _end);
	uart_puts_with_term("\n");
	uart_puts_with_term("_endof_sram:");
	uart_uint32_with_term((uint32_t) _endof_sram);
	uart_puts_with_term("\n");
	unsigned char *prev_heap_end;

	/* initialize */
	if (heap_end == 0) {
		heap_end = (unsigned char*) _heap_start;
	}
	prev_heap_end = heap_end;

#if 1
	if (heap_end + incr > stack_ptr) {
		/* heap overflow  */
		uart_puts_with_term("Heap Overflow\r\n");
		return (caddr_t) -1;
	}
#endif

	heap_end += incr;
	/*
	 #if 1 // Debug
	 //char buff[32];
	 //sprintf(buff,"incr:%d, heap:%x\n", incr, heap_end);
	 //UART_Send(Uart, (uint8_t*)buff, strlen(buff), BLOCKING);
	 endif*/
	return (caddr_t) prev_heap_end;
}
int _fstat_r(struct _reent *r, int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}
int _open_r(struct _reent *r, const char *path, int flags, int mode) {
	return -1;
}

int _isatty(int fd) {
	return 1;
}

char *__exidx_start;
char *__exidx_end;
