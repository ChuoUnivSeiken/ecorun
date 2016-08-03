/*
 * command.h
 *
 *  Created on: 2014/06/19
 *      Author: Yoshio
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "../system/common_types.h"
#include "../system/systimer.h"

#define USE_FREE_RTOS 1

#if USE_FREE_RTOS
#include "FreeRTOS.h"
#include "queue.h"
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#define MAX_COMMAND 20
#define MAX_COMMAND_QUEUE 4
#define MAX_COMMAND_DATA_SIZE 128

typedef struct command_arg_t
{
	const_string arg_value;
	//uint32_t arg_value_length;
	struct command_arg_t* next;
} command_arg;

// containing command and parameters
typedef struct command_data_t
{
	int command_id;
	int datasize;
	uint8_t data[MAX_COMMAND_DATA_SIZE];
	command_arg args[10];
	int args_count;
	systime_t accept_time;
	struct command_data_t* next;
} command_data;

typedef void (*command_func)(command_data*);

typedef void (*error_func)(const char* cmd, void (*func)(command_data*));

// first initialization
void initialize_command_system(error_func func);

typedef uint32_t command_id;

command_id register_command(const_string cmd, command_func func);
command_id get_command_id(const_string cmd);
uint32_t get_registered_command_count(void);
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
