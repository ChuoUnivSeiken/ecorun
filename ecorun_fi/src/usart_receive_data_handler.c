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

// see system/peripheral/usart_handler.c
void usart_receive_data_handler(string buf, uint32_t count)
{
	//systime_t receive_time = systimer_tick();

	if (strlen(buf) == 0)
	{
		return;
	}

	volatile string last;
	const_string cmd_name = strtok_r(buf, " ", (char**) &last);
	if (cmd_name == NULL)
	{
		usart_write_string("msg <invalid_command : ");
		usart_write_string(buf);
		usart_writeln_string(">");
		return;
	}
	uint32_t cmd_id = get_command_id(cmd_name);
	if (cmd_id == -1)
	{
		usart_write_string("msg <not registerd command : ");
		usart_write_string(buf);
		usart_writeln_string(">");
		return;
	}

	command_data* cmd = create_command();
	if (cmd == NULL)
	{
		usart_write_string("not accepted : ");
		usart_write_string(buf);
		usart_write_string(". ");
		usart_writeln_string("command queue is fully used.");
		return;
	}
	cmd->command_id = cmd_id;
	//cmd->accept_time = receive_time;
	strcpy(cmd->data, last);
	cmd->args_count = 0;

	volatile string arg_str;
	while ((arg_str = strtok_r(last, " ", (char**) &last)) != NULL)
	{
		command_arg* arg = &cmd->args[cmd->args_count++];
		arg->arg_value = arg_str;
	}
	enqueue_command(cmd);
}
