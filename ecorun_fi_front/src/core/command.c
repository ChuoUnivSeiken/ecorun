/*
 * command.c
 *
 *  Created on: 2014/06/19
 *      Author: Yoshio
 */

#include <stddef.h>
#include <string.h>
#include "command.h"
#include "../system/cmsis/LPC13Uxx.h"

static command_data command_data_reserved[MAX_COMMAND_QUEUE];
static command_data* command_data_recycle;
static command_data* commands;
static command_data* commands_last;
static volatile int registered_command_count = 0;
static volatile int queue_command_count = 0;
static error_func err_func = NULL;

typedef struct command_identity
{
	const char* command_name;
	command_func command_func;
} command_identity;

static command_identity command_table[MAX_COMMAND];

command_id register_command(const_string cmd, command_func func)
{
	if (registered_command_count < MAX_COMMAND)
	{
		command_table[registered_command_count].command_name = cmd;
		command_table[registered_command_count].command_func = func;
		return registered_command_count++;
	}
	err_func(cmd, func);
	return -1;
}
command_id get_command_id(const_string cmd)
{
	int i;
	for (i = 0; i < registered_command_count; i++)
	{
		if (strcmp(command_table[i].command_name, cmd) == 0)
		{
			return i;
		}
	}
	return -1;
}
uint32_t get_registered_command_count()
{
	return registered_command_count;
}
void execute_command(command_data* cmd)
{
	command_table[cmd->command_id].command_func(cmd);
	delete_command(cmd);
}
void execute_all_command(void)
{
	volatile command_data* cmd = dequeue_command();
	while (cmd != NULL)
	{
		execute_command(cmd);
		cmd = dequeue_command();
	}
}
void execute_one_command(void)
{
	command_data* cmd = dequeue_command();
	if (cmd != NULL && cmd->command_id >= 0
			&& cmd->command_id < registered_command_count)
	{
		execute_command(cmd);
	}
}

void initialize_command_system(error_func func)
{
	int i;
	commands = NULL;
	commands_last = NULL;
	command_data_recycle = command_data_reserved;
	for (i = 0; i < MAX_COMMAND_QUEUE; i++)
	{
		if (i == MAX_COMMAND_QUEUE - 1)
		{
			command_data_reserved[i].next = NULL;
		}
		else
		{
			command_data_reserved[i].next = &command_data_reserved[i + 1];
		}
	}
	err_func = func;
}

command_data* dequeue_command(void)
{
	if (commands == NULL)
	{
		return NULL;
	}
	volatile command_data* cmd = commands;
	commands = commands->next;
	cmd->next = NULL;
	if (commands == NULL)
	{
		commands_last = NULL;
	}
	queue_command_count = queue_command_count - 1;
	return cmd;
}

void enqueue_command(command_data* command)
{
	if (command == NULL)
	{
		return;
	}

	queue_command_count = queue_command_count + 1;
	if (commands == NULL)
	{
		commands = command;
		commands_last = command;
		return;
	}
	commands_last->next = command;
	commands_last = command;
}

command_data* create_command(void)
{
	if (command_data_recycle == NULL)
		return NULL;
	command_data* cmd = command_data_recycle;
	command_data_recycle = command_data_recycle->next;
	cmd->datasize = 0;
	cmd->command_id = -1;
	cmd->next = NULL;

	return cmd;
}

void delete_command(command_data* command)
{
	if (command == NULL)
		return;
	command->next = command_data_recycle;
	command_data_recycle = command;
}

uint32_t get_queue_command_count(void)
{
	return queue_command_count;
}

