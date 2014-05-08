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

enum storage_type
{
	STORAGETYPE_INMEMORY,
	STORAGETYPE_COUNT
}
typedef enum storage_type storage_type_t;

/**
@brief		Storage handler structure.
*/
struct storage_handler
{
	// TODO: function pointers or if statements?
};
typedef struct storage_handler storage_handler_t;

/**
@brief		Read data from the storage medium into memory.
@param		handler
			A pointer to the storage handler that controls how
			to read and write data in a particular storage medium.
@param		data
			A pointer to the byte array to read data into. Must be
			preallocated to the correct size.
@param		offset
			The relative offset from which to start reading
			within the storage medium.
@return		A status variable describing the outcome of the call.
@todo		Should probably have a case where it doesn't return read into
		data, but instead modifies the pointer. Useful when requesting
		data from a memory store, no need to write twice.
*/
status_t
storage_read(
	storage_handler_t	*handler,
	byte			*data,
	offset_t		offset
);

/**
@brief		Write data from memory into a storage medium.
@param		handler
			A pointer to the storage handler that controls how
			to read and write data in a particular storage medium.
@param		data
			A pointer to the data to be written to storage.
@param		offset
			The relative offset from which to start writing
			within the storage medium.
@return		A status variable describing the outcome of the call.
*/
status_t
storage_write(
	storage_handler	*handler,
	byte		*data,
	offset_t	offset
);

#ifdef __cplusplus
}
#endif

#endif
