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

static volatile const accessible_data_entry* registered = NULL;
static volatile uint32_t registered_count = 0;

void register_data(const accessible_data_entry* data, uint32_t count)
{
	registered = data;
	registered_count = count;
}

bool find_data(const_string id, accessible_data_entry* out)
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

	return false;
}

static bool is_first;

void usart_write_json_start(void)
{
	usart_write_string("{");
	is_first = true;
}

void usart_write_json_name(const char* name)
{
	if (!is_first)
	{
		usart_write_string(",");
	}

	is_first = false;

	usart_write_string("\"");
	usart_write_string(name);
	usart_write_string("\":");
}

void usart_write_json_string(const char* name, const char* value)
{
	usart_write_json_name(name);
	usart_write_string("\"");
	usart_write_string(value);
	usart_write_string("\"");
}

void usart_write_json_uint32(const char* name, uint32_t value)
{
	usart_write_json_name(name);
	usart_write_uint32(value);
}

void usart_write_json_binary(const char* name, const char* value, size_t size)
{
	usart_write_json_name(name);
	usart_write_string("\"");
	usart_write_base64(value, size);
	usart_write_string("\"");
}

void usart_write_json_end(void)
{
	usart_writeln_string("}");
}

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

extern xSemaphoreHandle usart_mutex;

void send_response_with_data(uint32_t status, uint32_t unique_id,
		const_string id, const_buffer data, size_t size)
{
	if (usart_mutex)
	{
		xSemaphoreTake(usart_mutex, portMAX_DELAY);
	}

	usart_write_json_start();
	{
		usart_write_json_string("subject", "response");
		usart_write_json_uint32("status", status);
		usart_write_json_uint32("unique-id", unique_id);

		// data field
		usart_write_json_string("id", id);
		usart_write_json_uint32("size", size);
		usart_write_json_binary("data", data, size);
		usart_write_json_uint32("checksum", adler32(data, size));
	}
	usart_write_json_end();

	if (usart_mutex)
	{
		xSemaphoreGive(usart_mutex);
	}
}

void send_response(uint32_t status, uint32_t unique_id)
{
	usart_write_json_start();
	{
		usart_write_json_string("subject", "response");
		usart_write_json_uint32("status", status);
		usart_write_json_uint32("unique-id", unique_id);
	}
	usart_write_json_end();
}
