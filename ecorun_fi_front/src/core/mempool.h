/*
 * mempool.h
 *
 *  Created on: 2015/04/08
 *      Author: Yoshio
 */

#ifndef CORE_MEMPOOL_H_
#define CORE_MEMPOOL_H_

#include "../system/common_types.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct mm_pool_segment_t
{
	uint8_t* buffer;
	struct mm_pool_segment_t* next;
} mm_pool_segment_t;

typedef struct
{
	uint32_t seg_size;
	uint32_t seg_count;
	uint8_t* mem;
	uint8_t* buffer_beg;
	mm_pool_segment_t* unused;
	mm_pool_segment_t* head;
} mm_pool_t;

typedef struct
{
	uint32_t size;
	mm_pool_segment_t* head;
	mm_pool_t* pool;
} mm_pointer;

bool mm_create_pool(uint32_t seg_size, uint32_t seg_count, mm_pool_t* new_pool);
mm_pointer mpool_obtain(size_t size, mm_pool_t *pool);
void mpool_trash(mm_pointer* ptr);

#if defined(__cplusplus)
}
#endif

#endif /* CORE_MEMPOOL_H_ */
