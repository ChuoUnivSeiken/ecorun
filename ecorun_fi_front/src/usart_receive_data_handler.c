/*
 * usart_receive_data_hander.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include <string.h>
#include "system/common_types.h"
#include "system/peripheral/usart.h"
#include "system/systimer.h"
#include "core/command.h"

#define RESPONSE_IMMEDIATELY 0

// see system/peripheral/usart_handler.c
void receive_request(const_string buf)
{
	command_data* cmd = create_command();
	if (cmd == NULL)
	{
		usart_write_string("not accepted : ");
		usart_write_string(buf);
		usart_write_string(". ");
		usart_writeln_string("command queue is fully used.");
		return;
	}

	if (sizeof(cmd->data) <= strlen(buf))
	{
		usart_write_string("msg <invalid_command : ");
		usart_write_string(buf);
		usart_write_string(". ");
		usart_writeln_string("data field is too long.");
		return;
	}

	cmd->command_id = 0;
	strcpy(cmd->data, buf);

#if RESPONSE_IMMEDIATELY
	execute_command(cmd);
#else
	enqueue_command(cmd);
#endif
}

void usart_receive_data_handler(string buf, uint32_t count)
{
	if (strlen(buf) == 0)
	{
		return;
	}

	receive_request(buf);
}
