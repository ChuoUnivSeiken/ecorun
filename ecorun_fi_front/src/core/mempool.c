/*
 * mempool.c
 *
 *  Created on: 2015/04/08
 *      Author: Yoshio
 */

#include "mempool.h"
#include <unistd.h>
#include <string.h>

// TODO: Must debug before using!
#include "../system/peripheral/usart.h"
#include "../util/usart_util.h"

bool mm_create_pool(uint32_t seg_size, uint32_t seg_count, mm_pool_t* new_pool)
{
	uint32_t total_size = (seg_size + sizeof(mm_pool_segment_t)) * seg_count;
	void* mem = sbrk(total_size);

	if (mem == NULL)
	{
		return false;
	}

	new_pool->mem = mem;
	new_pool->buffer_beg = mem + sizeof(mm_pool_segment_t) * seg_count;
	memset(mem, 0, total_size);
	new_pool->unused = mem;
	new_pool->head = NULL;

	new_pool->seg_size = seg_size;
	new_pool->seg_count = seg_count;

	mm_pool_segment_t* pointer = new_pool->unused;
	for (size_t index = 0; index < seg_count - 1; index++)
	{
		pointer->next = &new_pool->unused[index + 1];
		pointer = &new_pool->unused[index + 1];
	}
	pointer = NULL;

	return true;
}

mm_pointer mpool_obtain(size_t size, mm_pool_t *pool)
{
	mm_pointer ptr;
	ptr.size = size;
	ptr.pool = pool;
	uint32_t seg_count = size / pool->seg_size + 1;

	mm_pool_segment_t* seg_ptr = NULL;
	while (seg_count-- > 0)
	{
		if (pool->unused == NULL)
		{
			return ptr;
		}
		if (ptr.head == NULL)
		{
			seg_ptr = ptr.head = pool->unused;
		}
		else
		{
			seg_ptr = seg_ptr->next = pool->unused;
		}
		seg_ptr->next = NULL;
		pool->unused = pool->unused->next;
	}
	return ptr;
}

void mpool_trash(mm_pointer* ptr)
{
	mm_pool_t* pool = ptr->pool;
	mm_pool_segment_t* seg_ptr = ptr->head;
	while (seg_ptr != NULL)
	{
		seg_ptr->next = pool->unused;
		pool->unused = seg_ptr;
		seg_ptr = seg_ptr->next;
	}
	ptr->head = NULL;
	ptr->pool = NULL;
	ptr->size = 0;
}
