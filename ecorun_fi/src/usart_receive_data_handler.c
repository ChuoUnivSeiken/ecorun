/*
 * usart_receive_data_hander.c
 *
 *  Created on: 2015/03/16
 *      Author: Yoshio
 */

#include <string.h>
#include "system/peripheral/usart.h"
#include "core/command.h"

// see system/peripheral/usart_handler.c
void usart_receive_data_handler(uint8_t* buf, uint32_t count)
{
	command_data* cmd;

	if (strlen(buf) == 0)
	{
		return;
	}
	int32_t cmd_length = get_command_length(buf, count);
	if (cmd_length <= 0)
	{
		usart_write_string("invalid_command : ");
		usart_write_string(buf);
		usart_writeln_string("\n");
		return;
	}
	uint32_t cmd_id = get_command_id_len(buf, cmd_length);
	if (cmd_id == -1)
	{
		usart_write_string("not registerd command : ");
		usart_write_string(buf);
		usart_writeln_string("\n");
		return;
	}

	cmd = create_command();
	if (cmd == NULL)
	{
		usart_write_string("not accepted : ");
		usart_write_string(buf);
		usart_write_string(". ");
		usart_write_string("command queue is fully used.");
		usart_writeln_string("\n");
		return;
	}
	cmd->command_id = cmd_id;
	strncpy(cmd->data, buf, count);
	volatile char* ptr = &cmd->data[cmd_length + 1];
	volatile char* end_args = &cmd->data[count];
	cmd->args_count = 0;
	volatile char* start_current_arg = ptr;
	while (ptr < end_args)
	{
		if (((*ptr) == ' ') || (*ptr == '\0') || (ptr == (end_args - 1)))
		{
			command_arg* arg = &cmd->args[cmd->args_count++];
			arg->arg_value = start_current_arg;
			arg->arg_value_length = ptr - start_current_arg;
			start_current_arg = ptr + 1;
		}
		ptr++;
	}
	enqueue_command(cmd);
}
