/*
 * uart.h
 *
 *  Created on: 2014/06/03
 *      Author: Yoshio
 */

#ifndef UART_H_
#define UART_H_

#define UART_STX 0x02
#define UART_ETX 0x03

void uart_init(uint32_t baudrate);

void uart_write_char(uint8_t c);

uint32_t uart_writeln_string(const uint8_t* s);
uint32_t uart_write_string(const uint8_t* s);

void uart_endln(void);

#endif /* UART_H_ */
