/******************************************************************************/
/**
@file		skip_list_store.h
@author		Kris Wallperington
@brief		Implementation of a Skiplist data store
*/
/******************************************************************************/
#if !defined(SKIP_LIST_STORE_H_)
#define SKIP_LIST_STORE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "skip_list_types.h"
/* #include <time.h> / * For random seed * / */

/**
@brief	  Initializes an in-memory skip_list.

@param	  skip_list
				Pointer to a skip_list instance to initialize
@param	  key_type
				Type of key used in this instance of a skip_list.
@param	  key_size
				Size of key in bytes.
@param	  value_size
				Size of value in bytes.
@param	  maxheight
				Maximum number of levels the skip_list will have.
@param	  pnum
				The numerator portion of the p value.
@param	  pden
				The denominator portion of the p value.
@return Status of initialization.
 */
err_t
sl_initialize(
	skiplist_t	*skiplist,
	key_type_t	key_type,
	int			key_size,
	int			value_size,
	int			maxheight,
	int			pnum,
	int			pden
);

/**
@brief	  Destroys the skip_list in memory.

@details	Destroys the skip_list in memory and frees the underlying structures.

@param	  skip_list
				The skip_list to be destroyed
@return	 Status of destruction.
 */
err_t
sl_destroy(
	skiplist_t *skiplist
);

/**
@brief	  Inserts a @p key @p value pair into the skip_list.

@details	Inserts a @p key @p value pair into the skip_list. The key and value
			are copied byte-for-byte as passed by the user. Duplicate inserts
			are implicitly supported.

@param	  skip_list
				The skip_list in which to insert
@param	  key
				The key to be insert
@param	  value
				The value to be insert
@return	 Status of insertion.
 */
err_t
sl_insert(
	skiplist_t	*skiplist,
	ion_key_t	key,
	ion_value_t value
);

/**
@brief	  Requests the @p value stored at the given @p key.

@details	Requests the @p value stored at the given @p key. The resultant
			value is then copied into the pointer provided by the user.

@param	  skip_list
				The skip_list in which to query
@param	  key
				The key to be found
@param	  value
				The container in which to put the resultant data
@return	 Status of query.
 */
err_t
sl_query(
	skiplist_t	*skiplist,
	ion_key_t	key,
	ion_value_t value
);

/**
@brief	  Updates the value stored at @p key with the new @p value.

@details	Updates the value stored at @p key with the new @p value. The given
			value is copied byte-for-byte into the malloc'd memory already
			stored at the key. If the @p key does not exist within the skip_list,
			the key/value pair is inserted into the skip_list instead.

@param	  skip_list
				The skip_list in which to update
@param	  key
				The key to find and update
@param	  value
				The new value to be updated to
@return	 Status of updating.
 */
err_t
sl_update(
	skiplist_t	*skiplist,
	ion_key_t	key,
	ion_value_t value
);

/**
@brief	  Attempts to delete all key/value pairs stored at the given @p key.

@details	Attempts to delete all key/value pairs stored at the given @p key.
			Returns "err_item_not_found" if the requested @p key is not in
			the skip_list, and "err_ok" if the deletion was successful. Any
			memory previously used for the deleted key/value pair(s) is freed.

@param	  skip_list
				The skip_list in which to delete from
@param	  key
				The key to delete
@return	 Status of deletion.
 */
err_t
sl_delete(
	skiplist_t	*skiplist,
	ion_key_t	key
);

/**
@brief	  Searches for a node with the given @p key. Used in conjunction with
			sl_query to perform key lookups.

@details	Searches for a node with the given @p key. Used in conjunction with
			sl_query to perform key lookups. Returns the first node containing
			the target key (if it exists), or the closest node less than the
			target key if it does not exist.
 */
sl_node_t *
sl_find_node(
	skiplist_t	*skiplist,
	ion_key_t	key
);

/**
@brief	  Generates a psuedo-random height, bounded within [0, maxheight). The
			generator is seeded using the current epoch time when the skip_list
			is initialized.

@param	  skip_list
				The skip_list to read level generation parameters from
@return	 A height.
 */
sl_level_t
sl_gen_level(
	skiplist_t *skiplist
);

/**
@brief	  Iterates through each level of a skip_list and prints out the content
			of each node in a meaningful way. Intended for debug use only.

@param	  skip_list
				The skip_list to print
 */
void
print_skiplist(
	skiplist_t *skiplist
);

#if defined(__cplusplus)
}
#endif

#endif /* SKIP_LIST_STORE_H_ */
