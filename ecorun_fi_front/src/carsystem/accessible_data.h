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

typedef struct
{
	const_string name;
	void* data_ptr;
	uint32_t data_size;

	// attributes
	bool is_read_only;
} accessible_data_entry;

bool find_and_put_data(const_string id);
bool find_data(const_string id, accessible_data_entry* out);

void send_response(uint32_t status, uint32_t unique_id);
void send_response_with_data(uint32_t status, uint32_t unique_id,
		const_string id, const_buffer data, size_t size);

// data pointer is shared
void register_data(const accessible_data_entry* data, uint32_t count);

#if defined(__cplusplus)
}
#endif

#endif /* CARSYSTEM_ACCESSIBLE_DATA_H_ */
