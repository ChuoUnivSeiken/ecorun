/*
 * command.h
 *
 *  Created on: 2014/06/19
 *      Author: Yoshio
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

#define MAX_COMMAND 20
#define MAX_COMMAND_QUEUE 5

typedef struct command_arg
{
	const char* arg_value;
	uint32_t arg_value_length;
	struct command_arg* next;
} command_arg;

// containing command and parameters
typedef struct command_data
{
	int command_id;
	int datasize;
	uint8_t data[512];
	command_arg args[10];
	int args_count;
	struct command_data* next;
} command_data;

typedef void (*command_func)(command_data*);

typedef void (*error_func)(const char* cmd, void (*func)(command_data*));

// first initialization
void initialize_command_system(error_func func);

// command table operations
int register_command(const char* cmd, command_func func);
int get_command_id(const char* cmd);
int get_command_id_len(const char* cmd, uint32_t cmd_length);
int get_registered_command_count(void);
void execute_all_command(void);
void execute_one_command(void);

// command queue operations
command_data* dequeue_command(void);
void enqueue_command(command_data* command);
command_data* create_command(void);
void delete_command(command_data* command);
uint32_t get_queue_command_count(void);

#if defined(__cplusplus)
}
#endif
#endif /* COMMAND_H_ */
