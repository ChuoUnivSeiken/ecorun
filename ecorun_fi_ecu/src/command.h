/*
 * command.h
 *
 *  Created on: 2014/06/19
 *      Author: Yoshio
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "type.h"

#define MAX_COMMAND 15
#define MAX_COMMAND_QUEUE 20

typedef struct command_arg {
	const char* arg_value;
	uint32_t arg_value_length;
} command_arg;

// containing command and parameters
typedef struct command_data {
	volatile int command_id;
	volatile int datasize;
	volatile char data[50];
	volatile command_arg args[10];
	volatile int args_count;
	volatile struct command_data* next;
} command_data;

// first initialization
void initialize_command_system(void);

// command table operations
int register_command(const char* cmd, void (*func)(command_data*));
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

#endif /* COMMAND_H_ */
