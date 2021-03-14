/**
 * recalloc
 * 
 * Author: Nergal
 * License: MIT
 */

#ifndef RECALLOC_INCLUDED
#	define RECALLOC_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>


/** 'recalloc'
 * because 'realloc' doesn't zero-memory like 'calloc' can.
 * zeroing is only if the new size is larger.
 * decreasing simply uses 'realloc' since it won't introduce garbage values.
 */
static void *recalloc(void *const arr, const size_t new_size, const size_t element_size, const size_t old_size)
{
	if( arr==NULL || old_size==0 )
		return calloc(new_size, element_size);
	
	uint8_t *const new_block = ( uint8_t* )realloc(arr, new_size * element_size);
	if( new_block==NULL )
		return NULL;
	
	if( old_size < new_size )
		memset(&new_block[old_size * element_size], 0, (new_size - old_size) * element_size);
	
	return new_block;
}

#ifdef __cplusplus
}
#endif

#endif /** RECALLOC_INCLUDED */