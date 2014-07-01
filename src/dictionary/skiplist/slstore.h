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
			are implicitly supported.

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

/**
@brief 		Requests the @p value stored at the given @p key.

@details 	Requests the @p value stored at the given @p key. The resultant
			value is then copied into the pointer provided by the user.

@param 		skiplist
				The skiplist in which to query
@param 		key
				The key to be found
@param 		value
				The container in which to put the resultant data
@return 	Status of query.
 */
err_t
sl_query(
	skiplist_t 		*skiplist,
	ion_key_t 		key,
	ion_value_t 	*value
);


/**
@brief 		Searches for a node with the given @p key. Used in conjunction with
			sl_query to perform key lookups.

@details 	Searches for a node with the given @p key. Used in conjunction with
			sl_query to perform key lookups. Returns the node with a key less
			than and closest to the target key.
 */
sl_node_t*
sl_find_node(
	skiplist_t 		*skiplist,
	ion_key_t 		key
);

/**
@brief 		Generates a psuedo-random height, bounded within [0, maxheight). The
			generator is seeded using the current epoch time when the skiplist
			is initialized.

@param 		skiplist
				The skiplist to read level generation parameters from
@return 	A height.
 */
sl_level_t
sl_gen_level(
	skiplist_t 		*skiplist
);

#ifdef __cplusplus
}
#endif

#endif /* SLSTORE_H */
