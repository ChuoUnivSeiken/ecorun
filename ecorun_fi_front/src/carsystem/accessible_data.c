/*
 * accessible_data.c
 *
 *  Created on: 2015/04/11
 *      Author: Yoshio
 */

#include <string.h>
#include "accessible_data.h"
#include "../system/peripheral/usart.h"
#include "../system/systimer.h"
#include "../util/usart_util.h"
#include "../core/adler32.h"
#include "../core/base64.h"

static volatile const named_data* registered = NULL;
static volatile uint32_t registered_count = 0;

void put_data(const_buffer data, size_t size, const_string id);

void register_data(const named_data* data, uint32_t count)
{
	registered = data;
	registered_count = count;
}

bool find_and_put_data(const_string id)
{
	volatile uint32_t i = 0;
	for (i = 0; i < registered_count; i++)
	{
		const_string name = registered[i].name;
		void* data_ptr = registered[i].data_ptr;
		size_t data_size = registered[i].data_size;
		if (strcmp(id, name) == 0)
		{
			put_data((const_buffer) data_ptr, data_size, name);
			return true;
		}
	}
}

bool find_data(const_string id, named_data* out)
{
	volatile uint32_t i = 0;
	for (i = 0; i < registered_count; i++)
	{
		const_string name = registered[i].name;
		if (strcmp(id, name) == 0)
		{
			*out = registered[i];
			return true;
		}
	}
}

/**
 * @remarks put (data)
 * 			data = (id 4bytes)(space 1byte)(base64 encoded binary nbytes)(checksum 4bytes)
 */
void put_data(const_buffer data, size_t size, const_string id)
{
	systime_t send_time = systimer_tick();
	usart_write_string("put ");
	usart_write_string(id);
	usart_write_string(" ");
	usart_write_uint32((uint32_t) size);
	usart_write_string(" ");
	usart_write_base64(data, size);
	usart_write_string(" ");
	usart_write_uint32(adler32(data, size));
	usart_write_string(" ");
	usart_write_uint32(send_time.high_part);
	usart_write_string(" ");
	usart_write_uint32(send_time.low_part);
	usart_writeln_string("");
}
