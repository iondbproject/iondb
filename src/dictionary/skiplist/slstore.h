/******************************************************************************/
/**
@file		slstore.h
@author		Kris Wallperington
@brief		Implementation of a Skiplist data store
*/
/******************************************************************************/
#ifndef SLSTORE_H
#define SLSTORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include "./../../io.h"
#include "./../../kv_system.h"

typedef int sl_level_t;

/**
@brief 	Struct of a node in the skiplist.
 */
typedef struct sl_node
{
	/* TODO unsure about these types for key and value */
	ion_key_t		key;
	ion_value_t		value;
	sl_level_t		height;
	struct sl_node	**next;
} sl_node_t;

/**
@brief 	Struct of the Skiplist, holds metadata and the entry point
 		into the skiplist.
 */
typedef struct skiplist
{
	sl_node_t	*head;
	sl_level_t	maxheight;
	int			key_size;
	int			value_size;
	int			pnum;
	int			pden;
} skiplist_t;

/**
@brief 		Initializes an in-memory skiplist.

@param 		skiplist
				Pointer to a skiplist instance to initialize
@param 		key_size
				Size of key in bytes.
@param 	 	value_size
				Size of value in bytes.
@param 		maxheight
				Maximum number of levels the skiplist will have.
@param 		pnum
				The numerator portion of the p value.
@param 		pden
				The denominator portion of the p value.
@return Status of initialization.
 */
err_t
sl_initialize(
	skiplist_t 	*skiplist,
	int 		key_size,
	int 		value_size,
	int 		maxheight,
	int 		pnum,
	int 		pden
);

/**
@brief 		Destroys the skiplist in memory.

@details 	Destroys the skiplist in memory and frees the underlying structures.

@param 		skiplist
				The skiplist to be destroyed
@return 	Status of destruction.
 */
err_t
sl_destroy(
	skiplist_t 		*skiplist
);

/**
@brief 		Inserts a @p key @p value pair into the skiplist.

@details 	Inserts a @p key @p value pair into the skiplist. The key and value
			are copied byte-for-byte as passed by the user. Duplicate inserts
			are undefined behavior (Until further notice) FIXME

@param 		skiplist
				The skiplist in which to insert
@param 		key
				The key to be insert
@param 		value
				The value to be insert
@return 	Status of insertion.
 */
err_t
sl_insert(
	skiplist_t 		*skiplist,
	ion_key_t 		key,
	ion_value_t 	value
);

sl_level_t
sl_gen_level(
	skiplist_t 		*skiplist
);

#ifdef __cplusplus
}
#endif

#endif /* SLSTORE_H */
