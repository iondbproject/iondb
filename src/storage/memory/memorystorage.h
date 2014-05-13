/******************************************************************************/
/**
@file		memorystorage.h
@author		Graeme Douglas
@brief		Memory storage medium API.
*/
/******************************************************************************/

#ifndef CHANGEME
#define CHANGEME

#ifdef __cplusplus
extern "C" {
#endif

#include "../storage.h"
#include "../chunktypes.h"
#include "../../commontypes.h"
#include <stdlib.h>

/**
@brief		Read data from memory chunk into byte array.
@param		handler
			A pointer to the chunk handler for the
			memory chunk to read from.
@param		data
			The array of bytes to read into.
@param		idx
			The index of the item to read from
			the chunk.
@returns	A status reporting the result of the call.
*/
status_t
memchunk_read(
	memchunk_handler_t	*handler,
	byte			*data,
	offset_t		idx
);

/**
@brief		Write data from a byte array to a memory chunk.
@param		handler
			A pointer to the chunk handler for the
			memory chunk to write to.
@param		data
			The array of bytes to read from.
@param		idx
			The index of the item location to
			write to.
@returns	A status reporting the result of the call.
*/
status_t
memchunk_write(
	memchunk_handler_t	*handler,
	byte			*data,
	offset_t		idx
);

/**
@brief		Initialize a segment of to be used for a memory chunk.
@param		segment
			A pointer to a byte pointer. The byte
			pointer will be set to an allocated space
			of memory.
@param		numitems
			The number of items the chunk should be
			able to store.
@param		itemsize
			The size of each item.
@returns	A status reporting the result of the call.
*/
status_t
memchunk_create_segment(
	byte			**segment,
	my_size_t		numitems,
	my_size_t		itemsize
);

/**
@brief		Initialize a memory chunk handler.
@details	The caller is responsible for allocating space
		for the handler either statically or dynamically.
@param		handler
			A pointer to the chunk handler for the
			memory chunk to initialize.
@param		segment
			The continuous chunk of memory that
			used as the storage medium.
@param		numitems
			The number of items the chunk should be
			able to store.
@param		itemsize
			The size of each item.
@returns	A status reporting the result of the call.
*/
status_t
memchunk_init_handler(
	memchunk_handler_t	*handler,
	byte			*segment,
	my_size_t		numitems,
	my_size_t		itemsize
);

// TODO: maybe don't need this?
/**
@brief		Close a memory chunk.
@details	This will free the memory chunk used, so make sure this is
		what you really want to do!
		
		The @c segment pointer of the chunk handler will be
		set to @c NULL.
@param		handler
			A pointer to the chunk handler for
			the memory chunk to free.
@returns	A status reporting the result of the call.
*/
status_t
memchunk_close(
	memchunk_handler_t	*handler
);

/**
@brief		Destroy a memory chunk.
@details	This will free the memory segment used, so make sure this is
		what you really want to do!
		
		The pointer pointed to by @p segment will be set to @c NULL.
@param		segment
			A pointer to a byte pointer. The memory pointed by
			the byte pointer will be de-allocated and then
			set to @c NULL.
@returns	A status reporting the result of the call.
*/
status_t
memchunk_destroy_segment(
	byte			**segment
);

#ifdef __cplusplus
}
#endif

#endif
