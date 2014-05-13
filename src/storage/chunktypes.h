/******************************************************************************/
/**
@file		chunktypes.h
@author		Graeme Douglas
@brief		Contains all chunk types.
@details	This file exists to avoid circular dependencies.
*/
/******************************************************************************/

#ifndef CHUNKTYPES_H
#define CHUNKTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../commontypes.h"

/**
@brief		Chunk type identifier.
*/
enum chunk_type
{
	CHUNKTYPE_INMEMORY,		/**< In memory chunk. */
	CHUNKTYPE_COUNT
};
typedef enum	chunk_type		chunk_type_t;

/**
@brief		Chunk handler structure.
*/
struct chunk_handler
{
	chunk_type_t	type;		/**< Type of chunk. Identifies the
					     underlying storage medium. */
	my_size_t	numitems;	/**< The number of items possible in
					     the chunk. */
	my_size_t	itemsize;	/**< The of each item in the chunk. */
};
typedef struct	chunk_handler		chunk_handler_t;

/**
@brief		Chunk handler structure for in-memory storage.
*/
struct memchunk_handler
{
	chunk_handler_t	super;
	byte		*segment;
};
typedef struct 	memchunk_handler	memchunk_handler_t;

#ifdef __cplusplus
}
#endif

#endif
