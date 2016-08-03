/*
 * syscall.c
 *
 *  Created on: 2014/06/27
 *      Author: Yoshio
 */

#define __CC_ARM
#include "../cmsis/LPC11xx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/reent.h>
#include "../peripheral/uart.h"
//#include "lpc17xx_uart.h"

#define ECHOBACK

unsigned char* heap_end;
register unsigned char* stack_ptr asm ("sp");

int _read_r(struct _reent *r, int file, char *ptr, int len)
{
	uart_writeln_string("_read_r\r\n");
	int i;
	unsigned char *p = (unsigned char*) ptr;

	for (i = 0; i < len; i++)
	{
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

int _lseek_r(struct _reent *r, int file, int ptr, int dir)
{
	uart_writeln_string("_lseek_r\r\n");
	return 0;
}

int _write_r(struct _reent *r, int file, const void *ptr, size_t len)
{
	uart_writeln_string("_write_r\r\n");
	int i;
	unsigned char *p = (unsigned char*) ptr;

	for (i = 0; i < len; i++)
	{
		if (*p == '\n')
		{
			//UART_Send(Uart, (uint8_t*) '\r', 1, BLOCKING);
		}
		//UART_Send(Uart, p++, 1, BLOCKING);
	}
	return len;
}

int _close_r(struct _reent *r, int file)
{
	uart_writeln_string("_close_r\r\n");
	return -1;
}

int _fstat_r(struct _reent *r, int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}
int _open_r(struct _reent *r, const char *path, int flags, int mode)
{
	return -1;
}

int _isatty(int fd)
{
	return 1;
}

char *__exidx_start;
char *__exidx_end;
