/*
 * accessible_data.h
 *
 *  Created on: 2015/04/11
 *      Author: Yoshio
 */

#ifndef CARSYSTEM_ACCESSIBLE_DATA_H_
#define CARSYSTEM_ACCESSIBLE_DATA_H_

#include "../system/common_types.h"
#include "../system/systimer.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct data_t
{
	const_string name;
	void* data_ptr;
	uint32_t data_size;

	// attributes
	bool is_read_only;
	//systime_t timestamp;
} named_data;

bool find_and_put_data(const_string id);
bool find_data(const_string id, named_data* out);

// data pointer is shared
void register_data(const named_data* data, uint32_t count);

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_ACCESSIBLE_DATA_H_ */
