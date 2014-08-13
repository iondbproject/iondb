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

#include "dicttypes.h"
#include "dictionary.h"

#include <time.h> /* For random seed */
#include "io.h"
#include "kv_system.h"

typedef int sl_level_t;

/**
@brief 	Struct of a node in the skiplist.
 */
typedef struct sl_node
{
	ion_key_t		key; 		/**< Key of a skiplist node */
	ion_value_t		value;		/**< Value of a skiplist node */
	sl_level_t		height;		/**< Height index of a skiplist node
									 (counts from 0) */
	struct sl_node	**next; 	/**< Array of nodes that form the next
	 	 	 	 	 	 	 	     column in the skiplist */
} sl_node_t;

/**
@brief 	Struct of the Skiplist, holds metadata and the entry point
 		into the skiplist.
 */
typedef struct skiplist
{
	/* TODO yuck */
	dictionary_parent_t
				super; 		/**< Parent structure holding dictionary level
							     information */
	sl_node_t	*head; 		/**< Entry point into the skiplist. Does not hold
	 	 	 	 	 	 	     any key/value information */
	sl_level_t	maxheight;	/**< Maximum height of the skiplist in terms of
	 	 	 	 	 	 	     the number of nodes */
	int			pnum;		/**< Probability NUMerator, used in height gen */
	int			pden;		/**< Probability DENominator, used in height gen */

							/**< Comparison function used to compare keys */
	char 		(*compare)(ion_key_t, ion_key_t, ion_key_size_t);
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
	key_type_t 	key_type,
	char 		(*)(ion_key_t, ion_key_t, ion_key_size_t),
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
@brief 		Updates the value stored at @p key with the new @p value.

@details 	Updates the value stored at @p key with the new @p value. The given
			value is copied byte-for-byte into the malloc'd memory already
			stored at the key. If the @p key does not exist within the skiplist,
			the key/value pair is inserted into the skiplist instead.

@param 		skiplist
				The skiplist in which to update
@param 		key
				The key to find and update
@param 		value
				The new value to be updated to
@return 	Status of updating.
 */
err_t
sl_update(
	skiplist_t 		*skiplist,
	ion_key_t 		key,
	ion_value_t 	value
);

/**
@brief 		Attempts to delete all key/value pairs stored at the given @p key.

@details 	Attempts to delete all key/value pairs stored at the given @p key.
			Returns "err_item_not_found" if the requested @p key is not in
			the skiplist, and "err_ok" if the deletion was successful. Any
			memory previously used for the deleted key/value pair(s) is freed.

@param 		skiplist
				The skiplist in which to delete from
@param 		key
				The key to delete
@return 	Status of deletion.
 */
err_t
sl_delete(
	skiplist_t 		*skiplist,
	ion_key_t 		key
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
