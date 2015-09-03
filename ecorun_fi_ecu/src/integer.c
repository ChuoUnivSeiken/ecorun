/*
 * integer.c
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#include "integer.h"
#include <string.h>

volatile static char num_convert_buf[10];

uint32_t int32_to_str(int32_t num, char* buf) {
	volatile int count = 0;
	if (num < 0) {
		buf[count++] = '-';
		num = -num;
	}

	count = uint32_to_str(num, &buf[count]);

	return count;
}

uint32_t uint32_to_str(uint32_t num, char* buf) {
	volatile int bufpos = 0;
	volatile int count = 0;
	if (num == 0) {
		strcpy(buf, "0");
		return 1;
	}

	while (num != 0) {
		num_convert_buf[bufpos++] = (num % 10) + '0';
		num = num / 10;
	}

	while (--bufpos >= 0) {
		buf[count++] = num_convert_buf[bufpos];
	}
	buf[count++] = '\0';

	return count;
}

char hex_table[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',
		'b', 'c', 'd', 'e', 'f' };

uint32_t uint32_to_hex_str(uint32_t num, char* buf) {
	volatile int bufpos = 0;
	volatile int count = 0;
	strcpy(buf, "0x");
	count = count + 2;

	while (bufpos < 8) {
		num_convert_buf[bufpos++] = hex_table[(num & 0x0f)];
		num = (num >> 4);
	}

	while (--bufpos >= 0) {
		buf[count++] = num_convert_buf[bufpos];
	}
	buf[count++] = '\0';

	return count;
}

uint32_t str_to_uint32(char* buf) {
	volatile uint32_t num = 0;
	volatile uint32_t base = 1;
	volatile char* ptr = buf;
	while (*(++ptr) != '\0')
		;

	while (ptr-- > buf) {
		num += (((*ptr) - '0') * base);
		base *= 10;
	}
	return num;
}

uint32_t str_to_uint32_len(const char* buf, uint32_t len) {
	volatile uint32_t num = 0;
	volatile uint32_t base = 1;
	volatile char* ptr = buf;
	volatile uint8_t* buf_end = buf + len;
	while (*(++ptr) != '\0' && ptr < buf_end)
		;

	while (ptr-- > buf) {
		num += (((*ptr) - '0') * base);
		base *= 10;
	}
	return num;
}

