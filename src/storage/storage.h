/******************************************************************************/
/**
@file		storage.h
@author		Graeme Douglas
@brief		Storage layer API.
*/
/******************************************************************************/
#ifndef STORAGE_H
#define STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "chunktypes.h"
#include "memory/memorystorage.h"
#include "../commontypes.h"

/**
@brief		Read data from the storage medium into memory.
@param		handler
			A pointer to the chunk handler that controls how
			to read and write data in a particular storage medium.
@param		data
			A pointer to the byte array to read data into. Must be
			preallocated to the correct size.
@param		idx
			The relative offset from which to start reading
			within the storage medium.
@return		A status variable describing the outcome of the call.
@todo		Should probably have a case where it doesn't return read into
		data, but instead modifies the pointer. Useful when requesting
		data from a memory store, no need to write twice.
*/
status_t
chunk_read(
	chunk_handler_t	*handler,
	byte		*data,
	offset_t	idx
);

/**
@brief		Write data from memory into a storage medium.
@param		handler
			A pointer to the chunk handler that controls how
			to read and write data in a particular storage medium.
@param		data
			A pointer to the data to be written to the chunk.
@param		idx
			The relative offset from which to start writing
			within the storage medium.
@return		A status variable describing the outcome of the call.
*/
status_t
chunk_write(
	chunk_handler_t	*handler,
	byte		*data,
	offset_t	idx
);

/**
@brief		Append data from memory at the end of a storage medium.
@details	Support for this operation depends on the underlying
		storage medium. Some storage mediums cannot support
		appends.
@param		handler
			A pointer to the chunk handler that controls how
			to read and write data in a particular storage medium.
@param		data
			A pointer to the data to be appended to the chunk.
@return		A status variable describing the outcome of the call.
*/
status_t
chunk_append(
	chunk_handler_t	*handler,
	byte		*data
);

#if 0
/**
@brief		Create a chunk to store data to.
@param		name
			The name of the chunk.
			@todo Document how it
			will be cut down according
			to configuration.
@param		type
			The type of chunk to initialize.
@param		numitems
			The maximum number of items to store in the chunk.
@param		itemsize
			The size of each item.
@return		A status reporting the outcome of the call.
*/
status_t
chunk_create(
	char		*name,
	chunk_type_t	type,
	my_size_t	numitems,
	my_size_t	itemsize
);

/**
@brief		Close an open chunk.
@param		handler
			A pointer to a chunk handler instance for the
			chunk that is to be closed.
@return		A status reporting the outcome of the call.
*/
status_t
chunk_close(
	chunk_handler_t	*handler
);

/**
@brief		Permanently delete a chunk.
@param		name
			A character array containing the name of the chunk.
*/
status_t
chunk_remove(
	char		*name
);
#endif

#ifdef __cplusplus
}
#endif

#endif
